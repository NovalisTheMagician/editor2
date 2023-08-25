#include "../gwindows.h"

#include <assert.h>

#include "../edit.h"

void EditorWindow(bool *p_open, struct EdState *state)
{
    static bool firstTime = true;

    igSetNextWindowSize((ImVec2){ 800, 600 }, ImGuiCond_FirstUseEver);
    igSetNextWindowPos((ImVec2){ 40, 40 }, ImGuiCond_FirstUseEver, (ImVec2){ 0, 0 });

    igPushStyleVar_Vec2(ImGuiStyleVar_WindowMinSize, (ImVec2){ 400, 300 });
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar;
    if(state->map.dirty)
        flags |= ImGuiWindowFlags_UnsavedDocument;

    if(igBegin("Editor", p_open, flags))
    {
        if(igShortcut(ImGuiMod_Ctrl | ImGuiKey_C, 0, 0))
            EditCopy(state);
        if(igShortcut(ImGuiMod_Ctrl | ImGuiKey_V, 0, 0))
            EditPaste(state);
        if(igShortcut(ImGuiMod_Ctrl | ImGuiKey_X, 0, 0))
            EditCut(state);
        if(igShortcut(ImGuiMod_Ctrl | ImGuiKey_Z, 0, 0))
            printf("Undo!\n");
        if(igShortcut(ImGuiMod_Ctrl | ImGuiKey_Y, 0, 0))
            printf("Redo!\n");

        if(igShortcut(ImGuiKey_V, 0, 0))
            state->data.selectionMode = MODE_VERTEX;
        if(igShortcut(ImGuiKey_L, 0, 0))
            state->data.selectionMode = MODE_LINE;
        if(igShortcut(ImGuiKey_S, 0, 0))
            state->data.selectionMode = MODE_SECTOR;

        igPushItemWidth(80);
        static const char *modeNames[] = { "Vertex", "Line", "Sector" };
        static const size_t numModes = COUNT_OF(modeNames);
        igCombo_Str_arr("Mode", &state->data.selectionMode, modeNames, numModes, 3);

        igSameLine(0, 16);
        igPushItemWidth(80);
        static const char *gridSizes[] = { "1", "2", "4", "8", "16", "32", "64", "128", "256", "512", "1024" };
        static const size_t numGrids = COUNT_OF(gridSizes);
        int gridSelection = log2(state->data.gridSize);
        igCombo_Str_arr("Gridsize", &gridSelection, gridSizes, numGrids, numGrids);
        state->data.gridSize = pow(2, gridSelection);

        igSameLine(0, 16);
        igPushItemWidth(80);
        igSliderFloat("Zoom", &state->data.zoomLevel, MIN_ZOOM, MAX_ZOOM, "%.2f", 0);

        igSameLine(0, 16);
        if(igButton("Reset Zoom", (ImVec2){ 0, 0 })) { state->data.zoomLevel = 1; }

        igSameLine(0, 16);
        if(igButton("Go To Origin", (ImVec2){ 0, 0 })) 
        {
            state->data.viewPosition = (ImVec2){ -state->gl.editorFramebufferWidth / 2, -state->gl.editorFramebufferHeight / 2 };
        }

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

            if(hovored)
            {
                int edX = relX, edSX = relX, edY = relY, edSY = relY;
                float edXf = relX, edYf = relY;
                ScreenToEditorSpaceGrid(state, &edSX, &edSY);
                ScreenToEditorSpace(state, &edX, &edY);
                ScreenToEditorSpacef(state, &edXf, &edYf);
#ifdef _DEBUG
                state->data.mx = edX;
                state->data.my = edY;
                state->data.mtx = edSX;
                state->data.mty = edSY;
#endif

                if(igIsMouseDragging(ImGuiMouseButton_Middle, 2))
                {
                    ImVec2 dragDelta;
                    igGetMouseDragDelta(&dragDelta, ImGuiMouseButton_Middle, 2);
                    state->data.viewPosition.x -= dragDelta.x;
                    state->data.viewPosition.y -= dragDelta.y;
                    igResetMouseDragDelta(ImGuiMouseButton_Middle);

                    igSetWindowFocus_Nil();
                }

                if(igIsMouseClicked_Bool(ImGuiMouseButton_Left, false)) 
                {
                    
                }

                if(igIsMouseClicked_Bool(ImGuiMouseButton_Right, false)) 
                {
                    size_t newVertex;
                    if(!EditGetVertex(state, (struct Vertex){ edSX, edSY }, &newVertex))
                    {
                        newVertex = EditAddVertex(state, (struct Vertex){ edSX, edSY });
                    }
                    if(state->data.lastVertForLine != -1 && newVertex != state->data.lastVertForLine)
                    {
                        ssize_t lIdx = EditAddLine(state, state->data.lastVertForLine, newVertex);
                        assert(lIdx >= 0);
                        size_t bufferIdx = state->data.numLinesInBuffer++;
                        state->data.lineBuffer[bufferIdx] = lIdx;
                    }
                    else if(newVertex != state->data.lastVertForLine)
                    {
                        state->data.startVertex = newVertex;
                    }

                    if(newVertex == state->data.startVertex && state->data.numLinesInBuffer >= 3)
                    {
                        ssize_t sIdx = EditAddSector(state, state->data.lineBuffer, state->data.numLinesInBuffer);
                        assert(sIdx >= 0);
                        state->data.numLinesInBuffer = 0;
                        state->data.lastVertForLine = -1;
                    }
                    else
                    {
                        state->data.lastVertForLine = newVertex;
                    }

                    igSetWindowFocus_Nil();
                }

                if(igIsMouseClicked_Bool(ImGuiMouseButton_Middle, false)) 
                {
                    igSetWindowFocus_Nil();
                }

                ImGuiKeyData *wheelData = igGetKeyData_Key(ImGuiKey_MouseWheelY);
                if(wheelData->AnalogValue != 0)
                {
                    state->data.zoomLevel += wheelData->AnalogValue * 0.05f;
                    if(state->data.zoomLevel < MIN_ZOOM)
                        state->data.zoomLevel = MIN_ZOOM;
                    if(state->data.zoomLevel > MAX_ZOOM)
                        state->data.zoomLevel = MAX_ZOOM;

                    float edXAfter = relX, edYAfter = relY;
                    ScreenToEditorSpacef(state, &edXAfter, &edYAfter);

                    state->data.viewPosition.x += (edXf - edXAfter) * state->data.zoomLevel;
                    state->data.viewPosition.y += (edYf - edYAfter) * state->data.zoomLevel;

                    igSetWindowFocus_Nil();
                }
            }

            if(focused)
            {
                if (igIsKeyPressed_Bool(ImGuiKey_Space, false))
                {
                    
                }
            }

            ResizeEditorView(state, clientArea.x, clientArea.y);
            igImage((ImTextureID)(intptr_t)state->gl.editorColorTexture, clientArea, (ImVec2){ 0, 0 }, (ImVec2){ 1, 1 }, (ImVec4){ 1, 1, 1, 1 }, (ImVec4){ 1, 1, 1, 0 });

            if(firstTime)
            {
                firstTime = false;
                state->data.viewPosition = (ImVec2){ -state->gl.editorFramebufferWidth / 2, -state->gl.editorFramebufferHeight / 2 };
            }
        }
        igEndChild();
    }
    igEnd();
    igPopStyleVar(1);
}
