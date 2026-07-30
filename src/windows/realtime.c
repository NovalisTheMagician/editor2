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

            ImGuiIO *ioptr = igGetIO_Nil();
            float dt = ioptr->DeltaTime;

            const real_t speed = 78;

            if(hovored)
            {
                bool shiftDown = igGetIO_Nil()->KeyShift;
                bool altDown = igGetIO_Nil()->KeyAlt;
                bool ctrlDown = igGetIO_Nil()->KeyCtrl;

                Vec3 cameraPosition = state->realtime.cameraPosition;
                Vec3 cameraDirection = state->realtime.cameraDirection;
                Vec3 cameraRight = state->realtime.cameraRight;
                //Vec3 cameraUp = vec3_cross(cameraRight, cameraDirection);
                Vec3 globalUp = { 0, 1, 0 };

                real_t pitchMax = PIHALF - deg2rad(1);

                if(igIsMouseDragging(ImGuiMouseButton_Right, 1))
                {
                    ImVec2 dragDelta;
                    real_t yaw = atan2(cameraDirection.z, cameraDirection.x);
                    real_t pitch = asin(cameraDirection.y);
                    igGetMouseDragDelta(&dragDelta, ImGuiMouseButton_Right, 1);
                    yaw += dragDelta.x * dt;
                    pitch += -dragDelta.y * dt;
                    igResetMouseDragDelta(ImGuiMouseButton_Right);
                    
                    if(pitch > pitchMax)
                        pitch = pitchMax;
                    else if(pitch < -pitchMax)
                        pitch = -pitchMax;

                    while(yaw >= PI2)
                        yaw -= PI2;
                    while(yaw < 0)
                        yaw += PI2;
                    
                    real_t xzLen = cos(pitch);
                    cameraDirection = vec3_normalize((Vec3){ xzLen*cos(yaw), sin(pitch), xzLen*sin(yaw) });
                    cameraRight = vec3_normalize((Vec3){ -cameraDirection.z, 0, cameraDirection.x });
                    cameraUp = vec3_normalize(vec3_cross(cameraRight, cameraDirection));
                }

                if(igIsKeyDown_Nil(ImGuiKey_W) && !ctrlDown)
                {
                    cameraPosition = vec3_add(cameraPosition, vec3_scale(cameraDirection, speed * dt));
                }
                if(igIsKeyDown_Nil(ImGuiKey_S) && !ctrlDown)
                {
                    cameraPosition = vec3_add(cameraPosition, vec3_scale(cameraDirection, -speed * dt));
                }
                if(igIsKeyDown_Nil(ImGuiKey_D) && !ctrlDown)
                {
                    cameraPosition = vec3_add(cameraPosition, vec3_scale(cameraRight, speed * dt));
                }
                if(igIsKeyDown_Nil(ImGuiKey_A) && !ctrlDown)
                {
                    cameraPosition = vec3_add(cameraPosition, vec3_scale(cameraRight, -speed * dt));
                }
                if(igIsKeyDown_Nil(ImGuiKey_Q) && !ctrlDown)
                {
                    cameraPosition = vec3_add(cameraPosition, vec3_scale(globalUp, speed * dt));
                }
                if(igIsKeyDown_Nil(ImGuiKey_E) && !ctrlDown)
                {
                    cameraPosition = vec3_add(cameraPosition, vec3_scale(globalUp, -speed * dt));
                }


                state->realtime.cameraRight = cameraRight;
                state->realtime.cameraDirection = cameraDirection;
                state->realtime.cameraPosition = cameraPosition;
            }

            if(focused)
            {

            }

            ResizeRealtimeView(state, clientArea.x, clientArea.y);
            igImage((ImTextureRef){ ._TexID = state->gl.realtimeColorTexture }, clientArea, (ImVec2){ 0, 1 }, (ImVec2){ 1, 0 });
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
