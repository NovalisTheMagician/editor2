#include "async_load.h"

#include <ftplib.h>

static pthread_mutex_t stopMutex = PTHREAD_MUTEX_INITIALIZER;

static bool LoadFromFs(pstring path, uint8_t **buffer, size_t *size)
{
    FILE *file = fopen(pstr_tocstr(path), "r");
    if(!file) return false;

    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    rewind(file);

    *buffer = malloc(*size);
    size_t readTotal = 0, readCurrent;
    while((readCurrent = fread((*buffer) + readTotal, 1, (*size) - readTotal, file)) > 0)
    {
        readTotal += readCurrent;
    }

    fclose(file);

    return true;
}

static bool LoadFromFtp(pstring path, uint8_t **buffer, size_t *size, void *ftpHandle)
{
    uint32_t fileSize = 0;
    if(!FtpSize(pstr_tocstr(path), &fileSize, FTPLIB_BINARY, ftpHandle))
    {
        printf("%s", FtpLastResponse(ftpHandle));
        return false;
    }
    *size = fileSize;

    netbuf *fileHandle;
    if(!FtpAccess(pstr_tocstr(path), FTPLIB_FILE_READ, FTPLIB_BINARY, ftpHandle, &fileHandle))
    {
        printf("%s", FtpLastResponse(ftpHandle));
        return false;
    }

    *buffer = malloc(*size);
    size_t readTotal = 0, readCurrent;
    while((readCurrent = FtpRead((*buffer) + readTotal, (*size) - readTotal, fileHandle)) > 0)
    {
        readTotal += readCurrent;
    }

    FtpClose(fileHandle);
    return true;
}

static void* ThreadFunction(void *data)
{
    struct AsyncJob *job = data;

    size_t locIdx = job->currentBatch * BATCH_SIZE, i = 0;
    bool run = true;
    while(run || i < BATCH_SIZE)
    {
        pthread_mutex_lock(&stopMutex);
        run = job->stopRequest;
        pthread_mutex_unlock(&stopMutex);

        size_t idx = job->batch.numBuffers++;
        job->batch.names[idx] = job->infos[locIdx].name;
        job->batch.mtimes[idx] = job->infos[locIdx].mtime;
        if(job->locationType == LOCATION_FS) // fs
        {
            if(!LoadFromFs(job->infos[locIdx].path, &job->batch.buffers[idx], &job->batch.bufferSizes[idx]))
                job->batch.numBuffers--;
        }
        else if(job->locationType == LOCATION_FTP) // ftp
        {
            if(!LoadFromFtp(job->infos[locIdx].path, &job->batch.buffers[idx], &job->batch.bufferSizes[idx], job->handle))
                job->batch.numBuffers--;
        }

        locIdx++;
        i++;
    }
    job->currentBatch++;
    job->done = true;

    return NULL;
}

bool Async_StartJobFs(struct AsyncJob *job, struct FetchLocation *fetchList, size_t len, batch_finish_cb finishCb, void *user)
{
    if(job->running) return false;

    job->running = true;
    job->done = false;
    job->stopRequest = false;

    job->finishCb = finishCb;
    job->infos = fetchList;
    job->numInfos = len;

    job->locationType = LOCATION_FS; // fs

    job->currentBatch = 0;
    job->totalBatches = (size_t)ceil((float)len / BATCH_SIZE);
    job->batch.numBuffers = false;

    job->user = user;

    pthread_create(&job->threadObj, NULL, ThreadFunction, job);

    return true;
}

bool Async_StartJobFtp(struct AsyncJob *job, struct FetchLocation *fetchList, size_t len, batch_finish_cb finishCb, void *ftpHandle, void *user)
{
    if(job->running) return false;

    job->running = true;
    job->done = false;
    job->stopRequest = false;

    job->finishCb = finishCb;
    job->infos = fetchList;
    job->numInfos = len;

    job->locationType = LOCATION_FTP; // ftp

    job->currentBatch = 0;
    job->totalBatches = (size_t)ceil((float)len / BATCH_SIZE);
    job->batch.numBuffers = false;

    job->handle = ftpHandle;

    job->user = user;

    pthread_create(&job->threadObj, NULL, ThreadFunction, job);

    return true;
}

static void freeBatch(struct Batch batch)
{
    for(size_t i = 0; i < batch.numBuffers; ++i)
        free(batch.buffers[i]);
}

static void freeFetches(struct FetchLocation *locations, size_t num)
{
    for(size_t i = 0; i < num; ++i)
    {
        pstr_free(locations[i].name);
        pstr_free(locations[i].path);
    }
}

void Async_UpdateJob(struct AsyncJob *job)
{
    if(job->stopRequest)
    {
        pthread_join(job->threadObj, NULL);
        job->finishCb(job->batch, true, job->locationType, job->handle, job->user);
        
        freeBatch(job->batch);
        freeFetches(job->infos, job->numInfos);
        free(job->infos);

        job->running = false;
        job->stopRequest = false;
        job->done = false;
    }
    else if(job->done)
    {
        pthread_join(job->threadObj, NULL);
        job->finishCb(job->batch, job->currentBatch == job->totalBatches, job->locationType, job->handle, job->user);
        freeBatch(job->batch);
        job->batch.numBuffers = 0;

        if(job->currentBatch == job->totalBatches)
        {
            freeFetches(job->infos, job->numInfos);
            free(job->infos);
            job->running = false;
            job->stopRequest = false;
            job->done = false;
        }
        else
        {
            job->done = false;
            pthread_create(&job->threadObj, NULL, ThreadFunction, job);
        }
    }
}

void Async_AbortJob(struct AsyncJob *job)
{
    if(!job->running) return;
    pthread_mutex_lock(&stopMutex);
    job->stopRequest = true;
    pthread_mutex_unlock(&stopMutex);
}
