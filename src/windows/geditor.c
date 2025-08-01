#include "../gwindows.h"

#include <assert.h>
#include <string.h>
#include "cimgui.h"

#include "editor.h"
#include "geometry.h"
#include "logging.h"
#include "map.h"
#include "utils.h"
#include "../edit.h"

#define DEFAULT_WHITE { 1, 1, 1, 1 }
#define LINE_DIST 10
#define VERTEX_DIST 5

static void SubmitEditData(EdState *state, bool isLoop)
{
    LogDebug("Edit Done with %d vertices", state->data.editVertexBufferSize);

    bool res;
    if(isLoop)
        res = EditApplySector(state, state->data.editVertexBufferSize, state->data.editVertexBuffer);
    else
        res = EditApplyLines(state, state->data.editVertexBufferSize, state->data.editVertexBuffer);

    if(!res)
        LogWarning("Failed to insert all lines, too many intersections");

    state->data.editVertexBufferSize = 0;
}

static bool within(vec2s min, vec2s max, vec2s v)
{
    return v.x >= min.x && v.y >= min.y && v.x <= max.x && v.y <= max.y;
}

static void RectSelect(EdState *state, bool add)
{
    vec2s min = { .x = min(state->data.startDrag.x, state->data.endDrag.x), .y = min(state->data.startDrag.y, state->data.endDrag.y) };
    vec2s max = { .x = max(state->data.startDrag.x, state->data.endDrag.x), .y = max(state->data.startDrag.y, state->data.endDrag.y) };

    LogDebug("min: %d %d", (int)min.x, (int)min.y);
    LogDebug("max: %d %d", (int)max.x, (int)max.y);

    if(!add)
        state->data.numSelectedElements = 0;

    switch(state->data.selectionMode)
    {
    case MODE_VERTEX:
        {
            for(MapVertex *vertex = state->map.headVertex; vertex; vertex = vertex->next)
            {
                if(within(min, max, vertex->pos))
                {
                    state->data.selectedElements[state->data.numSelectedElements++] = vertex;
                }
            }
        }
        break;
    case MODE_LINE:
        {
            for(MapLine *line = state->map.headLine; line; line = line->next)
            {
                if(within(min, max, line->a->pos) && within(min, max, line->b->pos))
                {
                    state->data.selectedElements[state->data.numSelectedElements++] = line;
                }
            }
        }
        break;
    case MODE_SECTOR:
        {
            for(MapSector *sector = state->map.headSector; sector; sector = sector->next)
            {
                bool allPointsIn = true;
                for(size_t i = 0; i < sector->numOuterLines; ++i)
                {
                    allPointsIn &= within(min, max, sector->outerLines[i]->a->pos);
                }

                if(allPointsIn)
                {
                    state->data.selectedElements[state->data.numSelectedElements++] = sector;
                }
            }
        }
        break;
    }
}

static void AddEditVertex(EdState *state, vec2s v)
{
    size_t idx = state->data.editVertexBufferSize++;
    state->data.editVertexBuffer[idx] = v;
}

void EditorWindow(bool *p_open, EdState *state)
{
    if(igShortcut_Nil(ImGuiMod_Ctrl | ImGuiKey_C, ImGuiInputFlags_RouteGlobal))
        EditCopy(state);
    if(igShortcut_Nil(ImGuiMod_Ctrl | ImGuiKey_V, ImGuiInputFlags_RouteGlobal))
        EditPaste(state);
    if(igShortcut_Nil(ImGuiMod_Ctrl | ImGuiKey_X, ImGuiInputFlags_RouteGlobal))
        EditCut(state);
    if(igShortcut_Nil(ImGuiMod_Ctrl | ImGuiKey_Z, ImGuiInputFlags_RouteGlobal))
        LogDebug("Undo!");
    if(igShortcut_Nil(ImGuiMod_Ctrl | ImGuiKey_Y, ImGuiInputFlags_RouteGlobal))
        LogDebug("Redo!");

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    if(state->map.dirty)
        flags |= ImGuiWindowFlags_UnsavedDocument;

    if(igShortcut_Nil(ImGuiKey_1, ImGuiInputFlags_RouteGlobal))
    {
        ChangeMode(state, MODE_VERTEX);
        igSetWindowFocus_Str("Editor");
    }
    if(igShortcut_Nil(ImGuiKey_2, ImGuiInputFlags_RouteGlobal))
    {
        ChangeMode(state, MODE_LINE);
        igSetWindowFocus_Str("Editor");
    }
    if(igShortcut_Nil(ImGuiKey_3, ImGuiInputFlags_RouteGlobal))
    {
        ChangeMode(state, MODE_SECTOR);
        igSetWindowFocus_Str("Editor");
    }

    if(igBegin("Editor", p_open, flags))
    {
        igPushItemWidth(80);
        static const char *modeNames[] = { "Vertex", "Line", "Sector", "Things" };
        static const size_t numModes = COUNT_OF(modeNames);
        int selectionMode = state->data.selectionMode;
        if(igCombo_Str_arr("Mode", &selectionMode, modeNames, numModes, numModes))
        {
            ChangeMode(state, selectionMode);
        }

        igSameLine(0, 16);
        igPushItemWidth(80);
        static const char *gridSizes[] = { "1", "2", "4", "8", "16", "32", "64", "128", "256", "512", "1024" };
        static const size_t numGrids = COUNT_OF(gridSizes);
        int gridSelection = log2f(state->data.gridSize);
        igCombo_Str_arr("Gridsize", &gridSelection, gridSizes, numGrids, numGrids);
        state->data.gridSize = powf(2, gridSelection);
        igSameLine(0, 16);
        igPushItemWidth(80);
        int altGridSelection = log2f(state->data.altGridSize);
        igCombo_Str_arr("Alt Gridsize", &altGridSelection, gridSizes, numGrids, numGrids);
        state->data.altGridSize = powf(2, altGridSelection);

        igSameLine(0, 16);
        igPushItemWidth(80);
        igSliderFloat("Zoom", &state->data.zoomLevel, MIN_ZOOM, MAX_ZOOM, "%.2f", 0);

        igSameLine(0, 16);
        if(igButton("Reset Zoom", (ImVec2){ 0, 0 })) { state->data.zoomLevel = 1; }

        igSameLine(0, 16);
        if(igButton("Go To Origin", (ImVec2){ 0, 0 }))
        {
            state->data.viewPosition = (ImVec2){ -state->gl.editorFramebufferWidth / 2.0f, -state->gl.editorFramebufferHeight / 2.0f };
        }

        igSameLine(0, 16);
        if(igButton("Go To", (ImVec2){ 0, 0 }))
        {

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

            float edX = relX, edSX = relX, edY = relY, edSY = relY;
            ScreenToEditorSpace(state, &edX, &edY);
            ScreenToEditorSpaceGrid(state, state->data.gridSize, &edSX, &edSY);

            bool shiftDown = igGetIO_Nil()->KeyShift;
            bool altDown = igGetIO_Nil()->KeyAlt;
            bool ctrlDown = igGetIO_Nil()->KeyCtrl;
            Map *map = &state->map;

            if(state->data.editState == ESTATE_ADDVERTEX)
            {
                if(altDown)
                {
                    MapLine *closestLine = EditGetClosestLine(map, (vec2s){ .x = edX, .y = edY }, LINE_DIST);
                    if(closestLine)
                    {
                        vec2s closestPoint = LineGetClosestPoint((line_t){ closestLine->a->pos, closestLine->b->pos }, (vec2s){ .x = edX, .y = edY });
                        edSX = closestPoint.x;
                        edSY = closestPoint.y;
                    }
                }
                else if(ctrlDown)
                {
                    MapVertex *closestVertex = EditGetClosestVertex(map, (vec2s){ .x = edX, .y = edY }, VERTEX_DIST + 5);
                    if(closestVertex)
                    {
                        edSX = closestVertex->pos.x;
                        edSY = closestVertex->pos.y;
                    }
                }
                else if(shiftDown)
                {
                    edSX = relX, edSY = relY;
                    ScreenToEditorSpaceGrid(state, state->data.altGridSize, &edSX, &edSY);
                }
            }

            if(hovored)
            {
#ifdef _DEBUG
                state->data.mx = edX;
                state->data.my = edY;
                state->data.mtx = edSX;
                state->data.mty = edSY;
#endif
                vec2s mouseVertex = { {edX, edY} };
                state->data.editVertexMouse = (vec2s){ .x = edSX, .y = edSY };
                state->data.editDragMouse = mouseVertex;
                if(state->data.editState == ESTATE_NORMAL)
                {
                    switch(state->data.selectionMode)
                    {
                    case MODE_VERTEX: state->data.hoveredElement = EditGetClosestVertex(map, mouseVertex, VERTEX_DIST); break;
                    case MODE_LINE: state->data.hoveredElement = EditGetClosestLine(map, mouseVertex, LINE_DIST); break;
                    case MODE_SECTOR: state->data.hoveredElement = EditGetSector(map, mouseVertex); break;
                    }
                }

                if(state->data.isDragging)
                {
                    state->data.endDrag = mouseVertex;

                    state->data.editVertexDrag[1].y = mouseVertex.y;
                    state->data.editVertexDrag[2].x = mouseVertex.x;
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

                if(igIsMouseDragging(ImGuiMouseButton_Left, 2) && state->data.editState != ESTATE_ADDVERTEX)
                {
                    igResetMouseDragDelta(ImGuiMouseButton_Left);
                    igSetWindowFocus_Nil();

                    if(!state->data.isDragging)
                    {
                        LogDebug("Start Drag");
                        state->data.isDragging = true;
                        state->data.startDrag = mouseVertex;
                        state->data.endDrag = mouseVertex;

                        state->data.editVertexDrag[0] = mouseVertex; // start position
                        state->data.editVertexDrag[1] = mouseVertex; // top right
                        state->data.editVertexDrag[2] = mouseVertex; // bottom left
                    }
                }
                else if(!igIsMouseDown_Nil(ImGuiMouseButton_Left))
                {
                    if(state->data.isDragging)
                    {
                        LogDebug("End Drag");
                        state->data.isDragging = false;
                        RectSelect(state, shiftDown);
                    }
                }

                if(igIsMouseClicked_Bool(ImGuiMouseButton_Left, false) && !state->data.isDragging)
                {
                    vec2s mouseVertexSnap = { {edSX, edSY} };
                    if(state->data.editState == ESTATE_ADDVERTEX)
                    {
                        if(state->data.editVertexBufferSize == 0)
                        {
                            AddEditVertex(state, mouseVertexSnap);
                        }
                        else
                        {
                            vec2s first = state->data.editVertexBuffer[0];
                            if(mouseVertexSnap.x == first.x && mouseVertexSnap.y == first.y && state->data.editVertexBufferSize >= 3)
                            {
                                // submit to edit
                                SubmitEditData(state, true);

                                state->data.editState = ESTATE_NORMAL;
                            }
                            else
                            {
                                vec2s last = state->data.editVertexBuffer[state->data.editVertexBufferSize-1];
                                if(!(mouseVertexSnap.x == last.x && mouseVertexSnap.y == last.y))
                                {
                                    AddEditVertex(state, mouseVertexSnap);
                                }
                            }
                        }
                        assert(state->data.editVertexBufferSize != EDIT_VERTEXBUFFER_CAP);
                    }
                    else if(state->data.editState == ESTATE_NORMAL)
                    {
                        void *selectedElement = NULL;
                        switch(state->data.selectionMode)
                        {
                        case MODE_VERTEX: selectedElement = EditGetClosestVertex(map, mouseVertex, VERTEX_DIST); break;
                        case MODE_LINE: selectedElement = EditGetClosestLine(map, mouseVertex, LINE_DIST); break;
                        case MODE_SECTOR: selectedElement = EditGetSector(map, mouseVertex); break;
                        }

                        if(selectedElement)
                        {
                            if(shiftDown)
                            {
                                bool removed = false;
                                for(size_t i = 0; i < state->data.numSelectedElements; ++i)
                                {
                                    if(state->data.selectedElements[i] == selectedElement)
                                    {
                                        memmove(state->data.selectedElements + i, state->data.selectedElements + i + 1, (state->data.numSelectedElements - (i+1)) * sizeof *state->data.selectedElements);
                                        state->data.numSelectedElements--;
                                        removed = true;
                                        break;
                                    }
                                }
                                if(!removed)
                                {
                                    state->data.selectedElements[state->data.numSelectedElements++] = selectedElement;
                                }
                            }
                            else
                            {
                                state->data.numSelectedElements = 1;
                                state->data.selectedElements[0] = selectedElement;
                            }
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
                    ScreenToEditorSpace(state, &edXAfter, &edYAfter);

                    state->data.viewPosition.x += (edX - edXAfter) * state->data.zoomLevel;
                    state->data.viewPosition.y += (edY - edYAfter) * state->data.zoomLevel;

                    igSetWindowFocus_Nil();
                }

                if(igIsKeyPressed_Bool(ImGuiKey_Space, false))
                {
                    switch(state->data.editState)
                    {
                    case ESTATE_NORMAL:
                    {
                        state->data.editState = ESTATE_ADDVERTEX;
                        state->data.numSelectedElements = 0;
                        state->data.hoveredElement = NULL;
                    }
                    break;
                    case ESTATE_ADDVERTEX:
                    {
                        if(state->data.editVertexBufferSize >= 2)
                        {
                            // submit edit data
                            SubmitEditData(state, false);
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

                if(igIsKeyPressed_Bool(ImGuiKey_Delete, false))
                {
                    if(state->data.editState == ESTATE_NORMAL)
                    {
                        switch(state->data.selectionMode)
                        {
                        case MODE_VERTEX: EditRemoveVertices(map, state->data.numSelectedElements, (MapVertex**)state->data.selectedElements); break;
                        case MODE_LINE: EditRemoveLines(map, state->data.numSelectedElements, (MapLine**)state->data.selectedElements); break;
                        case MODE_SECTOR: EditRemoveSectors(map, state->data.numSelectedElements, (MapSector**)state->data.selectedElements); break;
                        }
                        state->data.numSelectedElements = 0;
                        state->data.hoveredElement = NULL;
                    }
                }

                if(igIsKeyPressed_Bool(ImGuiKey_Escape, false))
                {
                    if(state->data.editState == ESTATE_ADDVERTEX)
                    {
                        state->data.editVertexBufferSize = 0;
                        state->data.editState = ESTATE_NORMAL;
                    }
                    else if(state->data.editState == ESTATE_NORMAL)
                    {
                        if(state->data.numSelectedElements > 0)
                            state->data.numSelectedElements = 0;
                    }
                }

                if(igIsKeyPressed_Bool(ImGuiKey_Tab, false))
                {
                    if(state->data.editState == ESTATE_ADDVERTEX)
                    {
                        if(state->data.editVertexBufferSize >= 2)
                        {
                            // submit edit data
                            SubmitEditData(state, true);
                        }
                        state->data.editVertexBufferSize = 0;
                        state->data.editState = ESTATE_NORMAL;
                    }
                }
            }

            if(focused)
            {

            }

            ResizeEditorView(state, clientArea.x, clientArea.y);
            igImage((ImTextureRef){ ._TexID = state->gl.editorColorTexture }, clientArea, (ImVec2){ 0, 0 }, (ImVec2){ 1, 1 });

            static bool firstTime = true;
            if(firstTime)
            {
                firstTime = false;
                state->data.viewPosition = (ImVec2){ -state->gl.editorFramebufferWidth / 2.0f, -state->gl.editorFramebufferHeight / 2.0f };
            }
        }
        igEndChild();
    }
    igEnd();
}
