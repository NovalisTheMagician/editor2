#include "../gwindows.h"

void LogsWindow(bool *p_open, struct EdState *state)
{
    struct LogBuffer *log = &state->log;

    igSetNextWindowSize((ImVec2){ 800, 600 }, ImGuiCond_FirstUseEver);
    igSetNextWindowPos((ImVec2){ 40, 40 }, ImGuiCond_FirstUseEver, (ImVec2){ 0, 0 });

    if(igBegin("Logs", p_open, 0))
    {
        if(igButton("Clear", (ImVec2){ 0, 0 })) LogClear(log);
        igSameLine(0, 4);
        igCheckbox("Autoscroll", &state->data.autoScrollLogs);
        if(igBeginChild_Str("logWindow", (ImVec2){ 0, 0 }, true, 0))
        {
            size_t numLogs = LogLength(log);
            for(size_t i = 0; i < numLogs; ++i)
            {
                pstring str = LogGet(log, i);
                const char *start = str.data;
                const char *end = str.data + str.size;
                igTextUnformatted(start, end);
            }

            if(state->data.autoScrollLogs && igGetScrollY() >= igGetScrollMaxY())
                igSetScrollHereY(1.0f);

            igEndChild();
        }
    }
    igEnd();
}
