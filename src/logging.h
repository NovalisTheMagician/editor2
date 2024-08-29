#pragma once

#include <stddef.h>
#include "utils/pstring.h"

typedef struct LogBuffer
{
    pstring *lines;
    size_t start, length;
} LogBuffer;

typedef enum LogSeverity
{
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
} LogSeverity;

void LogInit(LogBuffer *logBuffer);
void LogDestroy(LogBuffer *logBuffer);

size_t LogLength(LogBuffer *logBuffer);
pstring LogGet(LogBuffer *logBuffer, size_t idx);
void LogClear(LogBuffer *logBuffer);
void LogString(LogBuffer *logBuffer, LogSeverity severity, pstring str);
void LogFormat(LogBuffer *logBuffer, LogSeverity severity, const char *format, ...);

void LogInfo(const char *format, ...);
void LogWarning(const char *format, ...);
void LogError(const char *format, ...);

#ifdef _DEBUG
void LogDebug(const char *format, ...);
#else
#define LogDebug(format, ...)
#endif
