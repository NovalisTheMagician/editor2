#pragma once

#include "common.h"

#include <pthread.h>

#define BATCH_SIZE 10

enum 
{
    LOCATION_FS,
    LOCATION_FTP
};

struct Batch
{
    uint8_t *buffers[BATCH_SIZE];
    size_t bufferSizes[BATCH_SIZE];
    pstring names[BATCH_SIZE];
    time_t mtimes[BATCH_SIZE];
    size_t numBuffers;
};

typedef void (*batch_finish_cb)(struct Batch batch, bool lastBatch, int type, void *handle, void *user);

struct FetchLocation
{
    pstring path;
    pstring name;
    time_t mtime;
};

struct AsyncJob
{
    bool running;
    bool stopRequest;
    bool done, batchDone;

    struct FetchLocation *infos;
    size_t numInfos;
    int locationType;

    void *handle;

    struct Batch batch;

    size_t currentBatch;
    size_t totalBatches;

    batch_finish_cb finishCb;
    pthread_t threadObj;

    void *user;
};

bool Async_StartJobFs(struct AsyncJob *job, struct FetchLocation *fetchList, size_t len, batch_finish_cb finishCb, void *user);
bool Async_StartJobFtp(struct AsyncJob *job, struct FetchLocation *fetchList, size_t len, batch_finish_cb finishCb, void *ftpHandle, void *user);
void Async_UpdateJob(struct AsyncJob *job);
void Async_AbortJob(struct AsyncJob *job);
void Async_AbortJobAndWait(struct AsyncJob *job);
bool Async_IsRunningJob(struct AsyncJob *job);
