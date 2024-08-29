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

void LogInit(LogBuffer logBuffer[static 1]);
void LogDestroy(LogBuffer logBuffer[static 1]);

size_t LogLength(LogBuffer logBuffer[static 1]);
pstring LogGet(LogBuffer logBuffer[static 1], size_t idx);
void LogClear(LogBuffer logBuffer[static 1]);
void LogString(LogBuffer logBuffer[static 1], LogSeverity severity, pstring str);
void LogFormat(LogBuffer logBuffer[static 1], LogSeverity severity, const char format[static 1], ...);

void LogInfo(const char format[static 1], ...);
void LogWarning(const char format[static 1], ...);
void LogError(const char format[static 1], ...);

#ifdef _DEBUG
void LogDebug(const char format[static 1], ...);
#else
#define LogDebug(format, ...)
#endif
