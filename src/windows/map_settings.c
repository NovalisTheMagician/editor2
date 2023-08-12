#include "../gwindows.h"

void MapSettingsWindow(bool *p_open, struct EdState *state)
{
    if(igBegin("Map Settings", p_open, 0))
    {
        igSliderInt("Texture Scale", &state->map.textureScale, 1, 10, NULL, 0);
    }
    igEnd();
}
