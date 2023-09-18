#pragma once

#include "common.h"

struct LogBuffer
{
    pstring *lines;
    size_t start, length;
};

enum LogSeverity
{
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
};

void LogInit(struct LogBuffer *logBuffer);
void LogDestroy(struct LogBuffer *logBuffer);

size_t LogLength(struct LogBuffer *logBuffer);
pstring LogGet(struct LogBuffer *logBuffer, size_t idx);
void LogClear(struct LogBuffer *logBuffer);
void LogString(struct LogBuffer *logBuffer, enum LogSeverity severity, pstring str);
void LogFormat(struct LogBuffer *logBuffer, enum LogSeverity severity, const char *format, ...);
