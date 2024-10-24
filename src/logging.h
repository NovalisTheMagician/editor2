#pragma once

#include <stddef.h>
#include "memory.h"

typedef enum LogSeverity
{
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
} LogSeverity;

typedef struct LogBuffer
{
    char **lines;
    LogSeverity *severities;
    size_t start, length;

    int capacity;
} LogBuffer;

void LogInit(LogBuffer *logBuffer);
void LogDestroy(LogBuffer *logBuffer);
void LogResizeBuffer(LogBuffer *logBuffer);

size_t LogLength(LogBuffer *logBuffer);
char* LogGet(LogBuffer *logBuffer, size_t idx);
LogSeverity LogGetSeverity(LogBuffer *logBuffer, size_t idx);
void LogClear(LogBuffer *logBuffer);
void LogString(LogBuffer *logBuffer, LogSeverity severity, const char *str);
void LogFormat(LogBuffer *logBuffer, LogSeverity severity, const char *format, ...);

void LogInfo(const char *format, ...);
void LogWarning(const char *format, ...);
void LogError(const char *format, ...);

#ifdef _DEBUG
void LogDebug(const char *format, ...);
#else
#define LogDebug(format, ...)
#endif
