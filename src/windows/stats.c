#include "../gwindows.h"

#include "../script.h"
#include "cimgui.h"

void StatsWindow(bool *p_open, EdState *state)
{
    if(igBegin("Debug", p_open, 0))
    {
        igSeparatorText("Editor");
        igText("Viewposition: %.2f | %.2f", state->data.viewPosition.x, state->data.viewPosition.y);
#ifdef _DEBUG
        igText("Mouse World Pos Snap: %d | %d", state->data.mtx, state->data.mty);
        igText("Mouse World Pos: %d | %d", state->data.mx, state->data.my);
#endif
        igText("Zoomlevel: %.2f", state->data.zoomLevel);
        igText("Num. Vertices: %d", state->map.vertexList.count);
        igText("Num. Lines: %d", state->map.lineList.count);
        igText("Num. Sectors: %d", state->map.sectorList.count);

        igSeparatorText("Plugins");
        if(igButton("Reload All", (ImVec2){ 0 })) ScriptReloadAll(&state->script);
        for(size_t i = 0; i < state->script.numPlugins; ++i)
        {
            igText("%s (%s)", state->script.plugins[i].name, state->script.plugins[i].file);
            igSameLine(0, 4);
            igPushID_Int(i);
            if(igButton("Reload", (ImVec2){ 0 }))
                ScriptReloadPlugin(&state->script, i);
            igPopID();
        }
    }
    igEnd();
}
