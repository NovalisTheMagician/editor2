#include "../gwindows.h"
#include "cimgui.h"

void MapSettingsWindow(bool *p_open, EdState *state)
{
    if(igBegin("Map Settings", p_open, ImGuiWindowFlags_NoDocking))
    {
        igSliderInt("Texture Scale", &state->map.textureScale, 1, 10, NULL, 0);
    }
    igEnd();
}
