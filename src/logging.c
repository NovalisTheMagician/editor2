#include "logging.h"
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <tgmath.h>
#include <time.h>

#include "memory.h" // IWYU pragma: keep

#define LOGBUFFER_CAPACITY 1024
#define LOGBUFFER_LINE_LEN 512

static LogBuffer *logBuffer_;

#ifdef _DEBUG
#include <stdio.h>
static FILE *logFile;
#endif

static size_t getNextIndex(LogBuffer *logBuffer)
{
    size_t idx = 0;
    if(logBuffer->length < (size_t)logBuffer->capacity)
    {
        idx = logBuffer->length;
        logBuffer->length++;
    }
    else
    {
        idx = logBuffer->start;
        logBuffer->start = (logBuffer->start + 1) % logBuffer->capacity;
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

void LogInit(LogBuffer *logBuffer)
{
    logBuffer->capacity = LOGBUFFER_CAPACITY;
    logBuffer->lines = calloc(logBuffer->capacity, sizeof *logBuffer->lines);
    logBuffer->severities = calloc(logBuffer->capacity, sizeof *logBuffer->severities);
    logBuffer->start = 0;
    logBuffer->length = 0;

    logBuffer_ = logBuffer;

#ifdef _DEBUG
    logFile = fopen("debug.log", "w");
#endif
}

void LogDestroy(LogBuffer *logBuffer)
{
    for(size_t i = 0; i < (size_t)logBuffer->capacity; ++i)
        free(logBuffer->lines[i]);
    free(logBuffer->lines);
    free(logBuffer->severities);
    logBuffer_ = NULL;

#ifdef _DEBUG
    fclose(logFile);
#endif
}

static void freeStrings(LogBuffer *logBuffer)
{
    for(size_t i = 0; i < logBuffer->length; ++i)
    {
        free(logBuffer->lines[i]);
        logBuffer->lines[i] = NULL;
    }
}

void LogResizeBuffer(LogBuffer *logBuffer)
{
    LogClear(logBuffer);
    freeStrings(logBuffer);
    logBuffer->lines = realloc(logBuffer->lines, logBuffer->capacity * sizeof *logBuffer->lines);
    logBuffer->severities = realloc(logBuffer->severities, logBuffer->capacity * sizeof *logBuffer->severities);
}

size_t LogLength(LogBuffer *logBuffer)
{
    return logBuffer->length;
}

char* LogGet(LogBuffer *logBuffer, size_t idx)
{
    idx += logBuffer->start;
    return logBuffer->lines[idx % logBuffer->capacity];
}

LogSeverity LogGetSeverity(LogBuffer *logBuffer, size_t idx)
{
    idx += logBuffer->start;
    return logBuffer->severities[idx % logBuffer->capacity];
}

void LogClear(LogBuffer *logBuffer)
{
    logBuffer->start = 0;
    logBuffer->length = 0;
}

void LogString(LogBuffer *logBuffer, LogSeverity severity, const char *str)
{
    size_t idx = getNextIndex(logBuffer);
    char *lineStr = logBuffer->lines[idx];
    if(lineStr == NULL)
        lineStr = calloc(LOGBUFFER_LINE_LEN, sizeof *lineStr);

    char timeBuffer[9] = { 0 };
    time_t timer = time(NULL);
    struct tm *tm_info = localtime(&timer);
    strftime(timeBuffer, sizeof timeBuffer, "%H:%M:%S", tm_info);

    snprintf(lineStr, LOGBUFFER_LINE_LEN, "[%s](%s): {%s}", timeBuffer, severityToString(severity), str);

    logBuffer->lines[idx] = lineStr;
    logBuffer->severities[idx] = severity;
}

static void LogFormatV(LogBuffer *logBuffer, LogSeverity severity, const char *format, va_list args)
{
    size_t idx = getNextIndex(logBuffer);
    char *lineStr = logBuffer->lines[idx];
    if(lineStr == NULL)
        lineStr = calloc(LOGBUFFER_LINE_LEN, sizeof *lineStr);

    char timeBuffer[9] = { 0 };
    time_t timer = time(NULL);
    struct tm *tm_info = localtime(&timer);
    strftime(timeBuffer, sizeof timeBuffer, "%H:%M:%S", tm_info);
    size_t prefixSize = snprintf(lineStr, LOGBUFFER_LINE_LEN, "[%s](%s): ", timeBuffer, severityToString(severity));
    vsnprintf(lineStr + prefixSize, LOGBUFFER_LINE_LEN - prefixSize, format, args);

    logBuffer->lines[idx] = lineStr;
    logBuffer->severities[idx] = severity;
}

void LogFormat(LogBuffer *logBuffer, LogSeverity severity, const char *format, ...)
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

    vfprintf(logFile, format, args);
    fprintf(logFile, "\n");
    fflush(logFile);

    va_end(args);
}
#endif
