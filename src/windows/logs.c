#include "../gwindows.h"
#include "cimgui.h"

#include <string.h>

#include "../script.h"
#include "editor.h"
#include "logging.h"

#define Col2ImVec4(c) (ImVec4){ c.r, c.g, c.b, c.a };

static ImVec4 severityToColor(EdState *state, LogSeverity severity)
{
    switch(severity)
    {
    case LOG_INFO: return Col2ImVec4(state->settings.colors[COL_LOG_INFO]);
    case LOG_WARN: return Col2ImVec4(state->settings.colors[COL_LOG_WARN]);
    case LOG_ERROR: return Col2ImVec4(state->settings.colors[COL_LOG_ERRO]);
    case LOG_DEBUG: return Col2ImVec4(state->settings.colors[COL_LOG_DEBU]);
    }
}

void LogsWindow(bool *p_open, EdState *state)
{
    static char inputBuffer[512] = { 0 };
    LogBuffer *log = &state->log;

    if(igBegin("Logs", p_open, 0))
    {
        if(igButton("Clear", (ImVec2){ 0, 0 })) LogClear(log);
        igSameLine(0, 4);
        igCheckbox("Autoscroll", &state->data.autoScrollLogs);
        igPushItemWidth(-1);
        if(igInputText("##scriptinput", inputBuffer, sizeof inputBuffer, ImGuiInputTextFlags_EnterReturnsTrue, NULL, NULL))
        {
            ScriptRunString(&state->script, inputBuffer);
            memset(inputBuffer, 0, sizeof inputBuffer);
        }
        igPopItemWidth();
        if(igBeginChild_Str("logWindow", (ImVec2){ 0, 0 }, ImGuiChildFlags_Border, 0))
        {
            size_t numLogs = LogLength(log);
            for(size_t i = 0; i < numLogs; ++i)
            {
                pstring str = LogGet(log, i);
                LogSeverity severity = LogGetSeverity(log, i);
                igPushStyleColor_Vec4(ImGuiCol_Text, severityToColor(state, severity));
                const char *start = str;
                const char *end = str + string_length(str);
                igTextUnformatted(start, end);
                igPopStyleColor(1);
            }

            if(state->data.autoScrollLogs && igGetScrollY() >= igGetScrollMaxY())
                igSetScrollHereY(1.0f);

        }
        igEndChild();
    }
    igEnd();
}
