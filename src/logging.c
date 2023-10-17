#include "logging.h"
#include <assert.h>
#include <stdarg.h>
#include <tgmath.h>

#define LOGBUFFER_CAPACITY 1024
#define LOGBUFFER_LINE_LEN 512

static struct LogBuffer *logBuffer_;

static size_t getNextIndex(struct LogBuffer *logBuffer)
{
    size_t idx = 0;
    if(logBuffer->length < LOGBUFFER_CAPACITY)
    {
        idx = logBuffer->length;
        logBuffer->length++;
    }
    else 
    {
        idx = logBuffer->start;
        logBuffer->start = (logBuffer->start + 1) % LOGBUFFER_CAPACITY;
    }
    return idx;
}

static const char* severityToString(enum LogSeverity severity)
{
    switch(severity)
    {
    case LOG_DEBUG: return "DEBUG";
    case LOG_INFO: return "INFO ";
    case LOG_WARN: return "WARN ";
    case LOG_ERROR: return "ERROR";
    }
    return "     ";
}

void LogInit(struct LogBuffer *logBuffer)
{
    logBuffer->lines = calloc(LOGBUFFER_CAPACITY, sizeof *logBuffer->lines);
    logBuffer->start = 0;
    logBuffer->length = 0;

    logBuffer_ = logBuffer;
}

void LogDestroy(struct LogBuffer *logBuffer)
{
    for(size_t i = 0; i < LOGBUFFER_CAPACITY; ++i)
        pstr_free(logBuffer->lines[i]);
    free(logBuffer->lines);
    logBuffer_ = NULL;
}

size_t LogLength(struct LogBuffer *logBuffer)
{
    return logBuffer->length;
}

pstring LogGet(struct LogBuffer *logBuffer, size_t idx)
{
    idx += logBuffer->start;

    return logBuffer->lines[idx % LOGBUFFER_CAPACITY];
}

void LogClear(struct LogBuffer *logBuffer)
{
    logBuffer->start = 0;
    logBuffer->length = 0;
}

void LogString(struct LogBuffer *logBuffer, enum LogSeverity severity, pstring str)
{
    size_t idx = getNextIndex(logBuffer);
    pstring lineStr = logBuffer->lines[idx];
    if(lineStr.capacity == 0)
        lineStr = pstr_alloc(LOGBUFFER_LINE_LEN);

    char timeBuffer[9] = { 0 };
    time_t timer = time(NULL);
    struct tm *tm_info = localtime(&timer);
    strftime(timeBuffer, sizeof timeBuffer, "%H:%M:%S", tm_info);

    pstr_format(&lineStr, "[{c}]({c}): {s}", timeBuffer, severityToString(severity), str);

    logBuffer->lines[idx] = lineStr;
}

static void LogFormatV(struct LogBuffer *logBuffer, enum LogSeverity severity, const char *format, va_list args)
{
    size_t idx = getNextIndex(logBuffer);
    pstring lineStr = logBuffer->lines[idx];
    if(lineStr.capacity == 0)
        lineStr = pstr_alloc(LOGBUFFER_LINE_LEN);

    char timeBuffer[9] = { 0 };
    time_t timer = time(NULL);
    struct tm *tm_info = localtime(&timer);
    strftime(timeBuffer, sizeof timeBuffer, "%H:%M:%S", tm_info);
    size_t prefixSize = pstr_format(&lineStr, "[{c}]({c}): ", timeBuffer, severityToString(severity));

    pstring payload = (pstring){ .data = lineStr.data + prefixSize, .size = 0, .capacity = lineStr.capacity - prefixSize };
    size_t payloadSize = pstr_vformat(&payload, format, args);

    lineStr.size += payloadSize;
    logBuffer->lines[idx] = lineStr;
}

void LogFormat(struct LogBuffer *logBuffer, enum LogSeverity severity, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    LogFormatV(logBuffer, severity, format, args);

    va_end(args);
}


void LogInfo(const char *format, ...)
{
    assert(logBuffer_);
    va_list args;
    va_start(args, format);

    LogFormatV(logBuffer_, LOG_INFO, format, args);

    va_end(args);
}

void LogWarning(const char *format, ...)
{
    assert(logBuffer_);
    va_list args;
    va_start(args, format);

    LogFormatV(logBuffer_, LOG_WARN, format, args);

    va_end(args);
}

void LogError(const char *format, ...)
{
    assert(logBuffer_);
    va_list args;
    va_start(args, format);

    LogFormatV(logBuffer_, LOG_ERROR, format, args);

    va_end(args);
}

#ifdef _DEBUG
void LogDebug(const char *format, ...)
{
    assert(logBuffer_);
    va_list args;
    va_start(args, format);

    LogFormatV(logBuffer_, LOG_DEBUG, format, args);

    va_end(args);
}
#endif
