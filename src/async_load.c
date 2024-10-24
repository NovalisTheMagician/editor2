#include "async_load.h"

#include <tgmath.h>

#include "memory.h" // IWYU pragma: keep

static pthread_mutex_t threadMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t batchMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t batchSignal = PTHREAD_COND_INITIALIZER;

static void* ThreadFunction(void *data)
{
    AsyncJob *job = data;

    bool run = true;
    while(true)
    {
        size_t locIdx = job->currentBatch * BATCH_SIZE, i = 0;
        while(run && i < BATCH_SIZE && locIdx < job->numInfos)
        {
            pthread_mutex_lock(&threadMutex);
            run = !job->stopRequest;
            pthread_mutex_unlock(&threadMutex);

            size_t idx = job->batch.numBuffers++;
            job->batch.names[idx] = job->infos[locIdx].name;
            job->batch.mtimes[idx] = job->infos[locIdx].mtime;

            if(!job->readCb(job->infos[locIdx].path, &job->batch.buffers[idx], &job->batch.bufferSizes[idx], job->handle))
                job->batch.numBuffers--;

            locIdx++;
            i++;
        }
        job->currentBatch++;
        pthread_mutex_lock(&threadMutex);
        job->batchDone = true;
        job->done = (job->currentBatch == job->totalBatches) || job->stopRequest;
        pthread_mutex_unlock(&threadMutex);

        if(job->done) break;

        pthread_cond_wait(&batchSignal, &batchMutex);
    }

    return NULL;
}

bool Async_StartJob(AsyncJob *job, FetchLocation *fetchList, size_t len, batch_finish_cb finishCb, read_cb readCb, finalize_cb finalizeCb, void *handle, void *user)
{
    if(job->running) return false;

    job->running = true;
    job->done = false;
    job->batchDone = false;
    job->stopRequest = false;

    job->finishCb = finishCb;
    job->infos = fetchList;
    job->numInfos = len;

    job->currentBatch = 0;
    job->totalBatches = (size_t)ceil((float)len / BATCH_SIZE);
    job->batch.numBuffers = 0;

    job->handle = handle;
    job->readCb = readCb;

    job->finalizeCb = finalizeCb;

    job->user = user;

    pthread_create(&job->threadObj, NULL, ThreadFunction, job);

    return true;
}

static void freeBatch(Batch batch)
{
    for(size_t i = 0; i < batch.numBuffers; ++i)
        free(batch.buffers[i]);
}

static void freeFetches(FetchLocation *locations, size_t num)
{
    for(size_t i = 0; i < num; ++i)
    {
        free(locations[i].name);
        free(locations[i].path);
    }
}

void Async_UpdateJob(AsyncJob *job)
{
    pthread_mutex_lock(&threadMutex);
    bool batchDone = job->batchDone;
    pthread_mutex_unlock(&threadMutex);
    if(batchDone)
    {
        job->finishCb(job->batch, job->done, job->handle, job->user);
        freeBatch(job->batch);
        job->batch.numBuffers = 0;

        if(job->done)
        {
            pthread_join(job->threadObj, NULL);
            freeFetches(job->infos, job->numInfos);
            free(job->infos);
            job->running = false;
            job->stopRequest = false;
            job->done = false;
            if(job->finalizeCb) job->finalizeCb(job->handle, job->user);
        }
        else
        {
            job->batchDone = false;
            pthread_cond_signal(&batchSignal);
        }
    }
}

void Async_AbortJob(AsyncJob *job)
{
    if(!job->running) return;
    pthread_mutex_lock(&threadMutex);
    job->stopRequest = true;
    pthread_mutex_unlock(&threadMutex);
}

void Async_AbortJobAndWait(AsyncJob *job)
{
    if(!job->running) return;

    pthread_mutex_lock(&threadMutex);
    job->stopRequest = true;
    pthread_mutex_unlock(&threadMutex);
    pthread_join(job->threadObj, NULL);
    if(job->finalizeCb) job->finalizeCb(job->handle, job->user);
    freeFetches(job->infos, job->numInfos);
    freeBatch(job->batch);
    free(job->infos);
}

bool Async_IsRunningJob(AsyncJob *job)
{
    return job->running;
}
