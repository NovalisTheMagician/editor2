#pragma once

#include "common.h"

struct LogBuffer
{
    pstring *lines;
    size_t start, length;
};

enum LogSeverity
{
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
};

void LogInit(struct LogBuffer logBuffer[static 1]);
void LogDestroy(struct LogBuffer logBuffer[static 1]);

size_t LogLength(struct LogBuffer logBuffer[static 1]);
pstring LogGet(struct LogBuffer logBuffer[static 1], size_t idx);
void LogClear(struct LogBuffer logBuffer[static 1]);
void LogString(struct LogBuffer logBuffer[static 1], enum LogSeverity severity, pstring str);
void LogFormat(struct LogBuffer logBuffer[static 1], enum LogSeverity severity, const char format[static 1], ...);

void LogInfo(const char format[static 1], ...);
void LogWarning(const char format[static 1], ...);
void LogError(const char format[static 1], ...);

#ifdef _DEBUG
void LogDebug(const char format[static 1], ...);
#else
#define LogDebug(format, ...) 
#endif
