#include "../gwindows.h"

#include <assert.h>

#include "../edit.h"

#define DEFAULT_WHITE { 1, 1, 1, 1 }

static void SubmitEditData(struct EdState *state)
{
    LogInfo("Edit Done with {d} vertices", state->data.editVertexBufferSize);

    EditApplySector(state, state->data.editVertexBuffer, state->data.editVertexBufferSize);

    state->data.editVertexBufferSize = 0;
}

static void AddEditVertex(struct EdState *state, struct Vertex v)
{
    size_t idx = state->data.editVertexBufferSize++;
    state->data.editVertexBuffer[idx] = v;
    state->gl.editorEdit.bufferMap[idx] = (struct VertexType) { .position = { v.x, v.y }, .color = DEFAULT_WHITE };
    state->gl.editorEdit.bufferMap[idx+1] = (struct VertexType) { .position = { v.x, v.y }, .color = DEFAULT_WHITE }; // set the next vertex to the same to remove line jerkiness
}

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
            LogInfo("Undo!");
        if(igShortcut(ImGuiMod_Ctrl | ImGuiKey_Y, 0, 0))
            LogInfo("Redo!");

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

            int edX = relX, edSX = relX, edY = relY, edSY = relY;
            float edXf = relX, edYf = relY;
            ScreenToEditorSpaceGrid(state, &edSX, &edSY);
            ScreenToEditorSpace(state, &edX, &edY);
            ScreenToEditorSpacef(state, &edXf, &edYf);

            if(hovored)
            {
#ifdef _DEBUG
                state->data.mx = edX;
                state->data.my = edY;
                state->data.mtx = edSX;
                state->data.mty = edSY;
#endif
                
                if(state->data.editState == ESTATE_ADDVERTEX)
                {
                    state->gl.editorEdit.bufferMap[state->data.editVertexBufferSize] = (struct VertexType) { .position = { edSX, edSY }, .color = DEFAULT_WHITE };
                }

                if(igIsMouseDragging(ImGuiMouseButton_Right, 2))
                {
                    ImVec2 dragDelta;
                    igGetMouseDragDelta(&dragDelta, ImGuiMouseButton_Right, 2);
                    state->data.viewPosition.x -= dragDelta.x;
                    state->data.viewPosition.y -= dragDelta.y;
                    igResetMouseDragDelta(ImGuiMouseButton_Right);

                    igSetWindowFocus_Nil();
                }

                if(igIsMouseClicked_Bool(ImGuiMouseButton_Left, false))
                {
                    struct Vertex mouseVertex = { edSX, edSY };
                    if(state->data.editState == ESTATE_ADDVERTEX)
                    {
                        if(state->data.editVertexBufferSize == 0)
                        {
                            AddEditVertex(state, mouseVertex);
                        }
                        else
                        {
                            struct Vertex first = state->data.editVertexBuffer[0];
                            if(mouseVertex.x == first.x && mouseVertex.y == first.y && state->data.editVertexBufferSize >= 3)
                            {
                                // submit to edit
                                SubmitEditData(state);

                                state->data.editState = ESTATE_NORMAL;
                            }
                            else
                            {
                                struct Vertex last = state->data.editVertexBuffer[state->data.editVertexBufferSize-1];
                                if(!(mouseVertex.x == last.x && mouseVertex.y == last.y))
                                {
                                    AddEditVertex(state, mouseVertex);
                                }
                            }
                        }
                        assert(state->data.editVertexBufferSize != EDIT_VERTEXBUFFER_CAP);
                    } 
                    else
                    {
                        size_t selectedSector;
                        if(EditGetSector(state, mouseVertex, &selectedSector))
                        {
                            LogInfo("Clicked on sector {d}", selectedSector);
                            EditRemoveSector(state, selectedSector);
                        }
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
                    state->data.zoomLevel = clamp(MIN_ZOOM, MAX_ZOOM, state->data.zoomLevel);

                    float edXAfter = relX, edYAfter = relY;
                    ScreenToEditorSpacef(state, &edXAfter, &edYAfter);

                    state->data.viewPosition.x += (edXf - edXAfter) * state->data.zoomLevel;
                    state->data.viewPosition.y += (edYf - edYAfter) * state->data.zoomLevel;

                    igSetWindowFocus_Nil();
                }

                if(igIsKeyPressed_Bool(ImGuiKey_Space, false))
                {
                    switch(state->data.editState)
                    {
                    case ESTATE_NORMAL:
                    {
                        state->data.editState = ESTATE_ADDVERTEX;
                        state->gl.editorEdit.bufferMap[0] = (struct VertexType) { .position = { edSX, edSY }, .color = DEFAULT_WHITE }; // set first vertex to mouse position to remove vertex jump
                    }
                    break;
                    case ESTATE_ADDVERTEX:
                    {
                        if(state->data.editVertexBufferSize >= 2)
                        {
                            // submit edit data
                            SubmitEditData(state);
                            state->data.editState = ESTATE_NORMAL;
                        }
                    }
                    break;
                    }
                }

                if(igIsKeyPressed_Bool(ImGuiKey_Backspace, false))
                {
                    if(state->data.editState == ESTATE_ADDVERTEX)
                    {
                        if(state->data.editVertexBufferSize > 0)
                        {
                            state->data.editVertexBufferSize--;
                        }
                    }
                }

                if(igIsKeyPressed_Bool(ImGuiKey_Escape, false))
                {
                    if(state->data.editState != ESTATE_NORMAL)
                    {
                        state->data.editVertexBufferSize = 0;
                        state->data.editState = ESTATE_NORMAL;
                    }
                }
            }

            if(focused)
            {
                
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
