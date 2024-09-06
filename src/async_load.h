#pragma once

#include <stdint.h>
#include <pthread.h>

#include "utils/pstring.h"

#define BATCH_SIZE 10

typedef struct Batch
{
    uint8_t *buffers[BATCH_SIZE];
    size_t bufferSizes[BATCH_SIZE];
    pstring names[BATCH_SIZE];
    time_t mtimes[BATCH_SIZE];
    size_t numBuffers;
} Batch;

typedef void (*batch_finish_cb)(Batch batch, bool lastBatch, void *handle, void *user);
typedef bool (*read_cb)(pstring, uint8_t**, size_t*, void*);

typedef struct FetchLocation
{
    pstring path;
    pstring name;
    time_t mtime;
} FetchLocation;

typedef struct AsyncJob
{
    bool running;
    bool stopRequest;
    bool done, batchDone;

    FetchLocation *infos;
    size_t numInfos;

    void *handle;

    Batch batch;

    size_t currentBatch;
    size_t totalBatches;

    batch_finish_cb finishCb;
    read_cb readCb;
    pthread_t threadObj;

    void *user;
} AsyncJob;

bool Async_StartJob(AsyncJob *job, FetchLocation *fetchList, size_t len, batch_finish_cb finishCb, read_cb readCb, void *handle, void *user);
void Async_UpdateJob(AsyncJob *job);
void Async_AbortJob(AsyncJob *job);
void Async_AbortJobAndWait(AsyncJob *job);
bool Async_IsRunningJob(AsyncJob *job);
