#pragma once

#include "common.h"

#include <pthread.h>

#define BATCH_SIZE 10

struct Batch
{
    uint8_t *buffers[BATCH_SIZE];
    size_t bufferSizes[BATCH_SIZE];
    pstring names[BATCH_SIZE];
    time_t mtimes[BATCH_SIZE];
    size_t numBuffers;
};

typedef void (*batch_finish_cb)(struct Batch batch, bool lastBatch, void *handle, void *user);
typedef bool (*read_cb)(pstring, uint8_t**, size_t*, void*);

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

    void *handle;

    struct Batch batch;

    size_t currentBatch;
    size_t totalBatches;

    batch_finish_cb finishCb;
    read_cb readCb;
    pthread_t threadObj;

    void *user;
};

bool Async_StartJob(struct AsyncJob *job, struct FetchLocation *fetchList, size_t len, batch_finish_cb finishCb, read_cb readCb, void *handle, void *user);
void Async_UpdateJob(struct AsyncJob *job);
void Async_AbortJob(struct AsyncJob *job);
void Async_AbortJobAndWait(struct AsyncJob *job);
bool Async_IsRunningJob(struct AsyncJob *job);
