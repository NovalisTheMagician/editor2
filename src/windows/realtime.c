#include "../gwindows.h"
#include "cimgui.h"

static vec3s screenToWorld(Vec2 screenCoord)
{
    return (vec3s){ 0 };
}

void RealtimeWindow(bool *p_open, EdState *state)
{
    if(igBegin("3D View", p_open, ImGuiWindowFlags_NoScrollbar))
    {
        if(igBeginChild_ID(1000, (ImVec2){ 0, 0 }, false, ImGuiWindowFlags_NoMove))
        {
            ImVec2 clientArea;
            igGetContentRegionAvail(&clientArea);

            ImVec2 clientPos;
            igGetWindowPos(&clientPos);

            bool hovored = igIsWindowHovered(0);
            bool focused = igIsWindowFocused(0);

            ImVec2 mpos;
            igGetMousePos(&mpos);
            int relX = (int)mpos.x - (int)clientPos.x;
            int relY = (int)mpos.y - (int)clientPos.y;
            (void)relX;
            (void)relY;

            if(hovored)
            {
                bool shiftDown = igGetIO_Nil()->KeyShift;
                bool altDown = igGetIO_Nil()->KeyAlt;

                if(igIsMouseDragging(ImGuiMouseButton_Right, 2))
                {

                }
            }

            if(focused)
            {

            }

            ResizeRealtimeView(state, clientArea.x, clientArea.y);
            igImage((ImTextureRef){ ._TexID = state->gl.realtimeColorTexture }, clientArea, (ImVec2){ 0, 0 }, (ImVec2){ 1, 1 });
        }
        igEndChild();

        state->ui.render3d = true;
    }
    else
    {
        state->ui.render3d = false;
    }
    igEnd();
}
