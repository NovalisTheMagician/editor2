#include "texture_load.h"

#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

#include <ftplib.h>
#include <pthread.h>

#include "logging.h"
#include "memory.h" // IWYU pragma: keep
#include "async_load.h"
#include "utils/string.h"

static bool ReadFromFs(const char *path, uint8_t **buffer, size_t *size, void *unused)
{
    (void)unused;

    FILE *file = fopen(path, "rb");
    if(!file) return false;

    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    rewind(file);

    *buffer = calloc(*size, sizeof **buffer);
    size_t readTotal = 0, readCurrent;
    while((readCurrent = fread((*buffer) + readTotal, 1, (*size) - readTotal, file)) > 0)
    {
        readTotal += readCurrent;
    }

    fclose(file);

    return true;
}

static bool ReadFromFtp(const char *path, uint8_t **buffer, size_t *size, void *ftpHandle)
{
    uint32_t fileSize = 0;
    if(!FtpSize(path, &fileSize, FTPLIB_BINARY, ftpHandle))
    {
        LogError("%s", FtpLastResponse(ftpHandle));
        return false;
    }
    *size = fileSize;

    netbuf *fileHandle;
    if(!FtpAccess(path, FTPLIB_FILE_READ, FTPLIB_BINARY, ftpHandle, &fileHandle))
    {
        LogError("%s", FtpLastResponse(ftpHandle));
        return false;
    }

    *buffer = calloc(*size, sizeof **buffer);
    size_t readTotal = 0, readCurrent;
    while((readCurrent = FtpRead((*buffer) + readTotal, (*size) - readTotal, fileHandle)) > 0)
    {
        readTotal += readCurrent;
    }

    FtpClose(fileHandle);
    return true;
}

static netbuf* ConnectToFtp(const char *url, const char *user, const char *pass)
{
    netbuf *handle;
    if(!FtpConnect(url, &handle))
    {
        LogError("failed to connect to ftp server: %s", url);
        return NULL;
    }

    if(!FtpLogin(user, pass, handle))
    {
        LogError("failed to login to ftp server: %s", user);
        FtpQuit(handle);
        return NULL;
    }

    return handle;
}

static pthread_mutex_t collectMutex = PTHREAD_MUTEX_INITIALIZER;
static bool cancelRequest;

static size_t CollectTexturesFtp(TextureCollection *tc, FetchLocation **locations, size_t size, size_t *capacity, char *folder, char *baseFolder, netbuf *ftpHandle)
{
    netbuf *dirHandle;
    if(!FtpAccess(folder, FTPLIB_DIR_VERBOSE, FTPLIB_ASCII, ftpHandle, &dirHandle))
    {
        LogError("%s:", FtpLastResponse(ftpHandle));
        LogError("failed to get dir listing: %s", folder);
        return 0;
    }

    size_t cap = 4096;
    struct
    {
        char *filePath;
        char *fileName;
        bool isDir;
    } *files = calloc(cap, sizeof *files);

    bool stopRequest = false;

    char buffer[1024] = { 0 };
    size_t numFiles = 0;
    ssize_t readLen;
    while((readLen = FtpRead(buffer, sizeof buffer, dirHandle)) > 0)
    {
        pthread_mutex_lock(&collectMutex);
        stopRequest = cancelRequest;
        pthread_mutex_unlock(&collectMutex);
        if(stopRequest) break;

        char *delim = strchr(buffer, ' ');
        *delim = '\0';
        char *perm = buffer;
        bool isDir = perm[0] == 'd';
        char *fn = NULL;
        while((delim = strchr(delim+1, ' ')) != NULL) fn = delim+1;
        if(fn == NULL)
        {
            LogWarning("failed to parse the filename out of a directory listing `%s`", buffer);
            continue;
        }

        size_t fnLen = strlen(fn);
        if(fn[fnLen-1] == '\n') fn[fnLen-1] = '\0';
        char *fileName = fn;
        char filePath[256] = { 0 };
        snprintf(filePath, sizeof filePath, "%s/%s", folder, fileName);

        files[numFiles++] = (typeof(*files)){ .fileName = CopyString(fileName), .filePath = CopyString(filePath), .isDir = isDir };
    }
    FtpClose(dirHandle);

    for(size_t i = 0; i < numFiles && !stopRequest; ++i)
    {
        pthread_mutex_lock(&collectMutex);
        stopRequest = cancelRequest;
        pthread_mutex_unlock(&collectMutex);

        if(files[i].isDir)
        {
            size = CollectTexturesFtp(tc, locations, size, capacity, files[i].filePath, baseFolder, ftpHandle);
            if(size >= *capacity)
            {
                size_t oldCapacity = *capacity;
                *capacity = (*capacity) * 2;
                *locations = realloc(*locations, (*capacity) * sizeof **locations);
                memset((*locations) + oldCapacity, 0, (*capacity) - oldCapacity);
            }
        }
        else
        {
            char *name = CopyString(files[i].filePath);
            char *ext = strrchr(name, '.');
            if(!ext) goto skipLocation;
            *ext = '\0';

            char timeBuffer[128] = { 0 };
            if(!FtpModDate(files[i].filePath, timeBuffer, sizeof timeBuffer, ftpHandle))
            {
                LogError("%s", FtpLastResponse(ftpHandle));
                goto skipLocation;
            }

            struct tm tm = { .tm_isdst = -1 };
            // 'YYYYMMDDHHMMSS'
#if defined(_WIN32)
            sscanf(timeBuffer, "%4d%2d%2d%2d%2d%2d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
#else
            strptime(timeBuffer, "%Y%m%d%H%M%S", &tm);
#endif
            time_t timestamp = mktime(&tm);

            if(!tc_is_newer(tc, name, timestamp))
            {
                goto skipLocation;
            }

            size_t idx = size++;
            (*locations)[idx] = (FetchLocation){ .name = CopyString(name), .path = CopyString(files[i].filePath), .mtime = timestamp };

            if(size >= *capacity)
            {
                size_t oldCapacity = *capacity;
                *capacity = (*capacity) * 2;
                *locations = realloc(*locations, (*capacity) * sizeof **locations);
                memset((*locations) + oldCapacity, 0, (*capacity) - oldCapacity);
            }

skipLocation:
            free(name);
        }
    }

    for(size_t i = 0; i < numFiles; ++i)
    {
        free(files[i].filePath);
        free(files[i].fileName);
    }
    free(files);
    return size;
}

static size_t CollectTexturesFs(TextureCollection *tc, FetchLocation **locations, size_t size, size_t *capacity, char *folder, char *baseFolder)
{
    DIR *dp = opendir(folder);
    if(!dp)
    {
        return 0;
    }

    bool stopRequest = false;

    struct dirent *entry;
    while((entry = readdir(dp)))
    {
        pthread_mutex_lock(&collectMutex);
        stopRequest = cancelRequest;
        pthread_mutex_unlock(&collectMutex);
        if(stopRequest) break;

        char *fileName = entry->d_name;
        if(strcmp(fileName, ".") == 0 || strcmp(fileName, "..") == 0) continue;

        char filePath[512] = { 0 };
        snprintf(filePath, sizeof filePath, "%s/%s", folder, fileName);

        struct stat buf;
        int r = stat(filePath, &buf);
        if(r == -1)
        {
            perror("stat");
            continue;
        }

        if(S_ISDIR(buf.st_mode))
        {
            size = CollectTexturesFs(tc, locations, size, capacity, filePath, baseFolder);
            if(size >= *capacity)
            {
                size_t oldCapacity = *capacity;
                *capacity = (*capacity) * 2;
                *locations = realloc(*locations, (*capacity) * sizeof **locations);
                memset((*locations) + oldCapacity, 0, (*capacity) - oldCapacity);
            }
        }
        else if(S_ISREG(buf.st_mode))
        {
            time_t timestamp = buf.st_mtime;
            char *name = CopyString(filePath);
            char *ext = strrchr(name, '.');
            if(ext)
            {
                *ext = '\0';
                if(!tc_is_newer(tc, name, timestamp)) goto skipLocation;

                size_t idx = size++;
                (*locations)[idx] = (FetchLocation){ .name = CopyString(name), .path = CopyString(filePath), .mtime = timestamp };

                if(size >= *capacity)
                {
                    size_t oldCapacity = *capacity;
                    *capacity = (*capacity) * 2;
                    *locations = realloc(*locations, (*capacity) * sizeof **locations);
                    memset((*locations) + oldCapacity, 0, (*capacity) - oldCapacity);
                }
            }
skipLocation:
            free(name);
        }
    }
    closedir(dp);

    return size;
}

static void BatchCallback(Batch batch, bool lastBatch, void *, void *user)
{
    EdState *state = user;
    TextureCollection *tc = &state->textures;

    for(size_t i = 0; i < batch.numBuffers; ++i)
    {
        if(!tc_load_mem(tc, batch.names[i], batch.buffers[i], batch.bufferSizes[i], batch.mtimes[i]))
        {
            LogError("failed to load %s\n", batch.names[i]);
        }
    }

    tc_sort(tc);

    if(lastBatch)
    {
        state->data.fetchingTextures = false;
    }
}

static void FinalizeFtpCallback(void *handle, void *)
{
    FtpQuit(handle);
}

typedef struct ThreadData
{
    EdState *state;
    char *folder;
} ThreadData;

static void freeFetches(FetchLocation *locations, size_t num)
{
    for(size_t i = 0; i < num; ++i)
    {
        free(locations[i].name);
        free(locations[i].path);
    }
}

void* LoadThread(void *user)
{
    ThreadData *data = user;
    Project *project = &data->state->project;
    TextureCollection *tc = &data->state->textures;
    AsyncJob *async = &data->state->async;
    char *textureFolder = data->folder;

    size_t capacity = 1024;
    FetchLocation *locations = calloc(capacity, sizeof *locations);

    bool cancel = false;
    size_t num = 0;
    if(project->basePath.type == ASSPATH_FTP)
    {
        netbuf *handle = ConnectToFtp(project->basePath.ftp.url, project->basePath.ftp.login, project->basePath.ftp.password);
        if(handle)
        {
            num = CollectTexturesFtp(tc, &locations, 0, &capacity, textureFolder, textureFolder, handle);
            pthread_mutex_lock(&collectMutex);
            cancel = cancelRequest;
            pthread_mutex_unlock(&collectMutex);
            if(num > 0 && !cancel) Async_StartJob(async, locations, num, BatchCallback, ReadFromFtp, FinalizeFtpCallback, handle, data->state);
            else
            {
                FtpQuit(handle);
                data->state->data.fetchingTextures = false;
            }
        }
        else
            data->state->data.fetchingTextures = false;
    }
    else
    {
        num = CollectTexturesFs(tc, &locations, 0, &capacity, textureFolder, textureFolder);
        pthread_mutex_lock(&collectMutex);
        cancel = cancelRequest;
        pthread_mutex_unlock(&collectMutex);
        if(num > 0 && !cancel) Async_StartJob(async, locations, num, BatchCallback, ReadFromFs, NULL, NULL, data->state);
        else data->state->data.fetchingTextures = false;
    }

    if(cancel)
    {
        freeFetches(locations, num);
        free(locations);
    }

    free(textureFolder);
    free(data);

    return NULL;
}

static pthread_t fetchThread;

void LoadTextures(EdState *state, bool refresh)
{
    CancelFetch();

    Project *project = &state->project;
    AsyncJob *async = &state->async;
    TextureCollection *tc = &state->textures;

    if(!refresh)
        tc_unload_all(tc);

    Async_AbortJob(async);

    char *textureFolder = calloc(512, sizeof *textureFolder);

    if(strlen(project->basePath.fs.path) == 0)
        strncpy(textureFolder, project->texturesPath, 256);
    else
        snprintf(textureFolder, 512, "%s/%s", project->basePath.fs.path, project->texturesPath);

    ThreadData *data = calloc(1, sizeof *data);
    *data = (ThreadData){ .state = state, .folder = textureFolder };

    state->data.fetchingTextures = true;

    cancelRequest = false;
    pthread_create(&fetchThread, NULL, LoadThread, data);
}

void CancelFetch(void)
{
    if(!fetchThread) return;
    pthread_mutex_lock(&collectMutex);
    cancelRequest = true;
    pthread_mutex_unlock(&collectMutex);
    pthread_join(fetchThread, NULL);
}
