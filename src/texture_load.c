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

    FILE *file = fopen(pstr_tocstr(path), "rb");
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
    if(!FtpSize(pstr_tocstr(path), &fileSize, FTPLIB_BINARY, ftpHandle))
    {
        LogError("{c}", FtpLastResponse(ftpHandle));
        return false;
    }
    *size = fileSize;

    netbuf *fileHandle;
    if(!FtpAccess(pstr_tocstr(path), FTPLIB_FILE_READ, FTPLIB_BINARY, ftpHandle, &fileHandle))
    {
        LogError("{c}", FtpLastResponse(ftpHandle));
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
    if(!FtpConnect(pstr_tocstr(url), &handle))
    {
        LogError("failed to connect to ftp server: {c}", pstr_tocstr(url));
        return NULL;
    }

    if(!FtpLogin(pstr_tocstr(user), pstr_tocstr(pass), handle))
    {
        LogError("failed to login to ftp server: {c}", pstr_tocstr(user));
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
    if(!FtpAccess(pstr_tocstr(folder), FTPLIB_DIR_VERBOSE, FTPLIB_ASCII, ftpHandle, &dirHandle))
    {
        LogError("{c}:", FtpLastResponse(ftpHandle));
        LogError("failed to get dir listing: {s}", pstr_tocstr(folder));
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

    pstring buffer = pstr_alloc(1024);
    size_t numFiles = 0;
    ssize_t readLen;
    while((readLen = FtpRead(buffer.data, buffer.capacity, dirHandle)) > 0)
    {
        pthread_mutex_lock(&collectMutex);
        stopRequest = cancelRequest;
        pthread_mutex_unlock(&collectMutex);
        if(stopRequest) break;

        buffer.size = readLen;
        pstring line = buffer;
        pstring perm = pstr_tok(&line, " ");
        pstr_tok(&line, " "); // inode refs
        pstr_tok(&line, " "); // user
        pstr_tok(&line, " "); // group
        pstr_tok(&line, " "); // size?
        pstr_tok(&line, " "); // day
        pstr_tok(&line, " "); // month
        pstr_tok(&line, " "); // time
        pstring fileName = pstr_tok(&line, " ");
        if(fileName.data[fileName.size - 1] == '\n') fileName.size--;
        pstring filePath = pstr_alloc(256);
        pstr_format(&filePath, "{s}/{s}", folder, fileName);

        bool isDir = perm.data[0] == 'd';

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
            ssize_t extIdx = pstr_last_index_of(files[i].filePath, ".");
            if(extIdx == -1) continue;
            pstring name = pstr_substring(files[i].filePath, baseFolder.size+1, extIdx);

            char timeBuffer[128] = { 0 };
            if(!FtpModDate(pstr_tocstr(files[i].filePath), timeBuffer, sizeof timeBuffer, ftpHandle))
            {
                LogError("{c}", FtpLastResponse(ftpHandle));
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
            (*locations)[idx] = (struct FetchLocation){ .name = pstr_copy(name), .path = pstr_copy(files[i].filePath), .mtime = timestamp };

            if(size >= *capacity) 
            {
                size_t oldCapacity = *capacity;
                *capacity = (*capacity) * 2;
                *locations = realloc(*locations, (*capacity) * sizeof **locations);
                memset((*locations) + oldCapacity, 0, (*capacity) - oldCapacity);
            }
        }
    }

    for(size_t i = 0; i < numFiles; ++i)
        pstr_free(files[i].filePath);
    free(files);
    pstr_free(buffer);
    return size;
}

static size_t CollectTexturesFs(struct TextureCollection *tc, struct FetchLocation **locations, size_t size, size_t *capacity, pstring folder, pstring baseFolder)
{
    DIR *dp = opendir(pstr_tocstr(folder));
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

        pstring fileName = pstr_cstr(entry->d_name);
        if(pstr_cmp(fileName, ".") == 0 || pstr_cmp(fileName, "..") == 0) continue;

        pstring filePath = pstr_alloc(256);
        pstr_format(&filePath, "{s}/{s}", folder, fileName);

        struct stat buf;
        int r = stat(pstr_tocstr(filePath), &buf);
        if(r == -1)
        {
            perror("stat");
            pstr_free(fileName);
            pstr_free(filePath);
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
            ssize_t extIdx = pstr_last_index_of(filePath, ".");
            if(extIdx != -1)
            {
                pstring name = pstr_substring(filePath, baseFolder.size+1, extIdx);
                if(!tc_is_newer(tc, name, timestamp)) continue;

                size_t idx = size++;
                (*locations)[idx] = (struct FetchLocation){ .name = pstr_copy(name), .path = pstr_copy(filePath), .mtime = timestamp };

                if(size >= *capacity) 
                {
                    size_t oldCapacity = *capacity;
                    *capacity = (*capacity) * 2;
                    *locations = realloc(*locations, (*capacity) * sizeof **locations);
                    memset((*locations) + oldCapacity, 0, (*capacity) - oldCapacity);
                }
            }
        }

        pstr_free(fileName);
        pstr_free(filePath);
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
            LogError("failed to load {c}\n", pstr_tocstr(batch.names[i]));
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
        pstr_free(locations[i].name);
        pstr_free(locations[i].path);
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

    pstr_free(textureFolder);
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

    pstring textureFolder = pstr_alloc(256);

    if(project->basePath.fs.path.size == 0)
        pstr_format(&textureFolder, "{s}", project->texturesPath);
    else
        pstr_format(&textureFolder, "{s}/{s}", project->basePath.fs.path, project->texturesPath);

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
