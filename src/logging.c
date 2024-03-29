#include "logging.h"
#include <assert.h>
#include <stdarg.h>
#include <tgmath.h>


#define LOGBUFFER_CAPACITY 1024
#define LOGBUFFER_LINE_LEN 512

static struct LogBuffer *logBuffer_;

#ifdef _DEBUG
#include <stdio.h>
static FILE *logFile;
#endif

static size_t getNextIndex(struct LogBuffer logBuffer[static 1])
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

void LogInit(struct LogBuffer logBuffer[static 1])
{
    logBuffer->lines = calloc(LOGBUFFER_CAPACITY, sizeof *logBuffer->lines);
    logBuffer->start = 0;
    logBuffer->length = 0;

    logBuffer_ = logBuffer;

#ifdef _DEBUG
    logFile = fopen("debug.log", "w");
#endif
}

void LogDestroy(struct LogBuffer logBuffer[static 1])
{
    for(size_t i = 0; i < LOGBUFFER_CAPACITY; ++i)
        string_free(logBuffer->lines[i]);
    free(logBuffer->lines);
    logBuffer_ = NULL;

#ifdef _DEBUG
    fclose(logFile);
#endif
}

size_t LogLength(struct LogBuffer logBuffer[static 1])
{
    return logBuffer->length;
}

pstring LogGet(struct LogBuffer logBuffer[static 1], size_t idx)
{
    idx += logBuffer->start;

    return logBuffer->lines[idx % LOGBUFFER_CAPACITY];
}

void LogClear(struct LogBuffer logBuffer[static 1])
{
    logBuffer->start = 0;
    logBuffer->length = 0;
}

void LogString(struct LogBuffer logBuffer[static 1], enum LogSeverity severity, pstring str)
{
    size_t idx = getNextIndex(logBuffer);
    pstring lineStr = logBuffer->lines[idx];
    if(lineStr == NULL)
        lineStr = string_alloc(LOGBUFFER_LINE_LEN);

    char timeBuffer[9] = { 0 };
    time_t timer = time(NULL);
    struct tm *tm_info = localtime(&timer);
    strftime(timeBuffer, sizeof timeBuffer, "%H:%M:%S", tm_info);

    string_format(lineStr, "[%s](%s): {%s}", timeBuffer, severityToString(severity), str);

    logBuffer->lines[idx] = lineStr;
}

static void LogFormatV(struct LogBuffer logBuffer[static 1], enum LogSeverity severity, const char format[static 1], va_list args)
{
    size_t idx = getNextIndex(logBuffer);
    pstring lineStr = logBuffer->lines[idx];
    if(lineStr == NULL)
        lineStr = string_alloc(LOGBUFFER_LINE_LEN);

    char timeBuffer[9] = { 0 };
    time_t timer = time(NULL);
    struct tm *tm_info = localtime(&timer);
    strftime(timeBuffer, sizeof timeBuffer, "%H:%M:%S", tm_info);
    size_t prefixSize = string_format(lineStr, "[%s](%s): ", timeBuffer, severityToString(severity));
    string_vformat_offset(lineStr, prefixSize, format, args);

    string_recalc(lineStr);
    logBuffer->lines[idx] = lineStr;
}

void LogFormat(struct LogBuffer logBuffer[static 1], enum LogSeverity severity, const char format[static 1], ...)
{
    va_list args;
    va_start(args, format);

    LogFormatV(logBuffer, severity, format, args);

    va_end(args);
}


void LogInfo(const char format[static 1], ...)
{
    assert(logBuffer_);
    va_list args;
    va_start(args, format);

    LogFormatV(logBuffer_, LOG_INFO, format, args);

    va_end(args);
}

void LogWarning(const char format[static 1], ...)
{
    assert(logBuffer_);
    va_list args;
    va_start(args, format);

    LogFormatV(logBuffer_, LOG_WARN, format, args);

    va_end(args);
}

void LogError(const char format[static 1], ...)
{
    assert(logBuffer_);
    va_list args;
    va_start(args, format);

    LogFormatV(logBuffer_, LOG_ERROR, format, args);

    va_end(args);
}

#ifdef _DEBUG
void LogDebug(const char format[static 1], ...)
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
