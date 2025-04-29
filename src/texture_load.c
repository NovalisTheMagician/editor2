#include "texture_load.h"

#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

#include <ftplib.h>
#include <pthread.h>

#include "logging.h"
#include "async_load.h"
#include "asset_sources/texture_load_fs.h"
#include "asset_sources/texture_load_ftp.h"
#include "utils/string.h"

pthread_mutex_t collectMutex = PTHREAD_MUTEX_INITIALIZER;
bool cancelRequest;

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

    NormalizePath(textureFolder);

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
