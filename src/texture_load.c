#include "texture_load.h"

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include <ftplib.h>
#include <pthread.h>

#include "async_load.h"

static bool ReadFromFs(pstring path, uint8_t **buffer, size_t *size, void *unused)
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

static bool ReadFromFtp(pstring path, uint8_t **buffer, size_t *size, void *ftpHandle)
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

static netbuf* ConnectToFtp(pstring url, pstring user, pstring pass)
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

static size_t CollectTexturesFtp(struct TextureCollection *tc, struct FetchLocation **locations, size_t size, size_t *capacity, pstring folder, pstring baseFolder, netbuf *ftpHandle)
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
        pstring filePath;
        pstring fileName;
        bool isDir;
    } *files = calloc(cap, sizeof *files);

    bool stopRequest = false;

    pstring buffer = string_alloc(1024);
    size_t numFiles = 0;
    ssize_t readLen;
    while((readLen = FtpRead(buffer, string_size(buffer), dirHandle)) > 0)
    {
        string_recalc(buffer);

        pthread_mutex_lock(&collectMutex);
        stopRequest = cancelRequest;
        pthread_mutex_unlock(&collectMutex);
        if(stopRequest) break;

        size_t numChars;
        struct stringtok *tok = stringtok_start(buffer);
        char *perm = stringtok_next(tok, " ", &numChars);
        bool isDir = perm[0] == 'd';
        stringtok_next(tok, " ", &numChars); // inode refs
        stringtok_next(tok, " ", &numChars); // user
        stringtok_next(tok, " ", &numChars); // group
        stringtok_next(tok, " ", &numChars); // size?
        stringtok_next(tok, " ", &numChars); // day
        stringtok_next(tok, " ", &numChars); // month
        stringtok_next(tok, " ", &numChars); // time
        char *fn = stringtok_next(tok, " ", &numChars);
        if(fn[numChars-1] == '\n') numChars--;
        pstring fileName = string_cstr_size(numChars, fn);
        pstring filePath = string_alloc(256);
        string_format(filePath, "%s/%s", folder, fileName);
        stringtok_end(tok);

        files[numFiles++] = (__typeof__(*files)){ .fileName = fileName, .filePath = filePath, .isDir = isDir };
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
            ssize_t extIdx = string_last_index_of(files[i].filePath, 0, ".");
            if(extIdx == -1) continue;
            pstring name = string_substring(files[i].filePath, string_length(baseFolder)+1, extIdx);

            char timeBuffer[128] = { 0 };
            if(!FtpModDate(files[i].filePath, timeBuffer, sizeof timeBuffer, ftpHandle))
            {
                LogError("%s", FtpLastResponse(ftpHandle));
                continue;
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
                continue;
            }

            size_t idx = size++;
            (*locations)[idx] = (struct FetchLocation){ .name = string_copy(name), .path = string_copy(files[i].filePath), .mtime = timestamp };

            if(size >= *capacity) 
            {
                size_t oldCapacity = *capacity;
                *capacity = (*capacity) * 2;
                *locations = realloc(*locations, (*capacity) * sizeof **locations);
                memset((*locations) + oldCapacity, 0, (*capacity) - oldCapacity);
            }

            string_free(name);
        }
    }

    for(size_t i = 0; i < numFiles; ++i)
        string_free(files[i].filePath);
    free(files);
    string_free(buffer);
    return size;
}

static size_t CollectTexturesFs(struct TextureCollection *tc, struct FetchLocation **locations, size_t size, size_t *capacity, pstring folder, pstring baseFolder)
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

        pstring filePath = string_alloc(256);
        string_format(filePath, "%s/%s", folder, fileName);

        struct stat buf;
        int r = stat(filePath, &buf);
        if(r == -1)
        {
            perror("stat");
            string_free(filePath);
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
            ssize_t extIdx = string_last_index_of(filePath, 0, ".");
            if(extIdx != -1)
            {
                pstring name = string_substring(filePath, string_length(baseFolder)+1, extIdx);
                if(!tc_is_newer(tc, name, timestamp)) continue;

                size_t idx = size++;
                (*locations)[idx] = (struct FetchLocation){ .name = string_copy(name), .path = string_copy(filePath), .mtime = timestamp };

                if(size >= *capacity) 
                {
                    size_t oldCapacity = *capacity;
                    *capacity = (*capacity) * 2;
                    *locations = realloc(*locations, (*capacity) * sizeof **locations);
                    memset((*locations) + oldCapacity, 0, (*capacity) - oldCapacity);
                }

                string_free(name);
            }
        }

        string_free(filePath);
    }
    closedir(dp);

    return size;
}

static void BatchCallback(struct Batch batch, bool lastBatch, void *handle, void *user)
{
    struct EdState *state = user;
    struct TextureCollection *tc = &state->textures;

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
        if(handle) FtpQuit(handle);
    }
}

struct ThreadData
{
    struct EdState *state;
    pstring folder;
};

static void freeFetches(struct FetchLocation *locations, size_t num)
{
    for(size_t i = 0; i < num; ++i)
    {
        string_free(locations[i].name);
        string_free(locations[i].path);
    }
}

void* LoadThread(void *user)
{
    struct ThreadData *data = user;
    struct Project *project = &data->state->project;
    struct TextureCollection *tc = &data->state->textures;
    struct AsyncJob *async = &data->state->async;
    pstring textureFolder = data->folder;

    size_t capacity = 1024;
    struct FetchLocation *locations = calloc(capacity, sizeof *locations);

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
            if(num > 0 && !cancel) Async_StartJob(async, locations, num, BatchCallback, ReadFromFtp, handle, data->state);
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
        if(num > 0 && !cancel) Async_StartJob(async, locations, num, BatchCallback, ReadFromFs, NULL, data->state);
        else data->state->data.fetchingTextures = false;
    }

    if(cancel)
    {
        freeFetches(locations, num);
        free(locations);
    }

    string_free(textureFolder);
    free(data);

    return NULL;
}

static pthread_t fetchThread;

void LoadTextures(struct EdState *state, bool refresh)
{
    CancelFetch();

    struct Project *project = &state->project;
    struct AsyncJob *async = &state->async;
    struct TextureCollection *tc = &state->textures;

    if(!refresh)
        tc_unload_all(tc);

    Async_AbortJob(async);

    pstring textureFolder = string_alloc(256);

    if(string_length(project->basePath.fs.path) == 0)
        string_format(textureFolder, "%s", project->texturesPath);
    else
        string_format(textureFolder, "%s/%s", project->basePath.fs.path, project->texturesPath);

    struct ThreadData *data = calloc(1, sizeof *data);
    *data = (struct ThreadData){ .state = state, .folder = textureFolder };
    
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
