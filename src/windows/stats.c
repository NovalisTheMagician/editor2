#include "../gwindows.h"

void StatsWindow(bool *p_open, EdState *state)
{
    if(igBegin("Debug Stats", p_open, 0))
    {
        igSeparatorText("Editor");
        igText("Viewposition: %.2f | %.2f", state->data.viewPosition.x, state->data.viewPosition.y);
#ifdef _DEBUG
        igText("Mouse World Pos Snap: %d | %d", state->data.mtx, state->data.mty);
        igText("Mouse World Pos: %d | %d", state->data.mx, state->data.my);
#endif
        igText("Zoomlevel: %.2f", state->data.zoomLevel);
        igText("Num. Vertices: %d", state->map.numVertices);
        igText("Num. Lines: %d", state->map.numLines);
        igText("Num. Sectors: %d", state->map.numSectors);
    }
    igEnd();
}
