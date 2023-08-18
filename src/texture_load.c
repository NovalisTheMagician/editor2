#include "texture_load.h"

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include <ftplib.h>
#include <pthread.h>

#include "async_load.h"

static netbuf* ConnectToFtp(pstring url, pstring user, pstring pass)
{
    netbuf *handle;
    if(!FtpConnect(pstr_tocstr(url), &handle))
    {
        printf("failed to connect to ftp server: %s\n", pstr_tocstr(url));
        return NULL;
    }

    if(!FtpLogin(pstr_tocstr(user), pstr_tocstr(pass), handle))
    {
        printf("failed to login to ftp server: %s\n", pstr_tocstr(user));
        FtpQuit(handle);
        return NULL;
    }
    
    return handle;
}

static size_t CollectTexturesFtp(struct TextureCollection *tc, struct FetchLocation **locations, size_t size, size_t *capacity, pstring folder, pstring baseFolder, netbuf *ftpHandle)
{
    netbuf *dirHandle;
    if(!FtpAccess(pstr_tocstr(folder), FTPLIB_DIR_VERBOSE, FTPLIB_ASCII, ftpHandle, &dirHandle))
    {
        printf("%s:", FtpLastResponse(ftpHandle));
        printf("failed to get dir listing: %s\n", pstr_tocstr(folder));
        return 0;
    }

    struct 
    {
        pstring filePath;
        pstring fileName;
        bool isDir;
    } *files = calloc(4096, sizeof *files);

    pstring buffer = pstr_alloc(1024);
    size_t numFiles = 0;
    ssize_t readLen;
    while((readLen = FtpRead(buffer.data, buffer.capacity, dirHandle)) > 0)
    {
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

    for(size_t i = 0; i < numFiles; ++i)
    {
        if(files[i].isDir)
        {
            size = CollectTexturesFtp(tc, locations, size, capacity, files[i].filePath, baseFolder, ftpHandle);
            if(size >= *capacity) 
            {
                *capacity = (*capacity) * 2;
                *locations = realloc(*locations, (*capacity) * sizeof **locations);
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
                printf("%s", FtpLastResponse(ftpHandle));
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
                pstr_free(files[i].filePath);
                continue;
            }

            size_t idx = size++;
            (*locations)[idx] = (struct FetchLocation){ .name = pstr_copy(name), .path = pstr_copy(files[i].filePath), .mtime = timestamp };

            if(size >= *capacity) 
            {
                *capacity = (*capacity) * 2;
                *locations = realloc(*locations, (*capacity) * sizeof **locations);
            }
            pstr_free(files[i].filePath);
        }
    }

    pstr_free(buffer);
    free(files);
    return size;
}

static size_t CollectTexturesFs(struct TextureCollection *tc, struct FetchLocation **locations, size_t size, size_t *capacity, pstring folder, pstring baseFolder)
{
    DIR *dp = opendir(pstr_tocstr(folder));
    if(!dp)
    {
        return 0;
    }

    struct dirent *entry;
    while((entry = readdir(dp)))
    {
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
                *capacity = (*capacity) * 2;
                *locations = realloc(*locations, (*capacity) * sizeof **locations);
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
                    *capacity = (*capacity) * 2;
                    *locations = realloc(*locations, (*capacity) * sizeof **locations);
                }
            }
        }

        pstr_free(fileName);
        pstr_free(filePath);
    }

    closedir(dp);

    return size;
}

static void BatchCallback(struct Batch batch, bool lastBatch, int type, void *handle, void *user)
{
    struct EdState *state = user;
    struct TextureCollection *tc = &state->textures;

    for(size_t i = 0; i < batch.numBuffers; ++i)
    {
        if(!tc_load_mem(tc, batch.names[i], batch.buffers[i], batch.bufferSizes[i], batch.mtimes[i]))
        {
            printf("failed to load %s\n", pstr_tocstr(batch.names[i]));
        }
    }

    tc_sort(tc);

    if(lastBatch)
    {
        state->data.fetchingTextures = false;
        if(type == LOCATION_FTP) FtpQuit(handle);
    }
}

struct ThreadData
{
    struct EdState *state;
    pstring folder;
};

void* LoadThread(void *user)
{
    struct ThreadData *data = user;
    struct Project *project = &data->state->project;
    struct TextureCollection *tc = &data->state->textures;
    struct AsyncJob *async = &data->state->async;
    pstring textureFolder = data->folder;

    size_t capacity = 1024;
    struct FetchLocation *locations = calloc(1024, sizeof *locations);

    if(project->basePath.type == ASSPATH_FTP)
    {
        netbuf *handle = ConnectToFtp(project->basePath.ftp.url, project->basePath.ftp.login, project->basePath.ftp.password);
        if(handle)
        {
            size_t num = CollectTexturesFtp(tc, &locations, 0, &capacity, textureFolder, textureFolder, handle);
            if(num > 0) Async_StartJobFtp(async, locations, num, BatchCallback, handle, data->state);
            else data->state->data.fetchingTextures = false;
        }
    }
    else
    {
        size_t num = CollectTexturesFs(tc, &locations, 0, &capacity, textureFolder, textureFolder);
        if(num > 0) Async_StartJobFs(async, locations, num, BatchCallback, data->state);
        else data->state->data.fetchingTextures = false;
    }

    pstr_free(textureFolder);
    free(data);

    return NULL;
}

void LoadTextures(struct EdState *state, bool refresh)
{
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

    pthread_t thread;
    pthread_create(&thread, NULL, LoadThread, data);
    pthread_detach(thread);
}
