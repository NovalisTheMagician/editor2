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
#include "vecmath.h"

#include "map/insert.h"

#define DEFAULT_WHITE { 1, 1, 1, 1 }
#define LINE_DIST fixed_from_int(10)
#define VERTEX_DIST fixed_from_int(5)

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

static bool within(FVec2 min, FVec2 max, FVec2 v)
{
    return v.x >= min.x && v.y >= min.y && v.x <= max.x && v.y <= max.y;
}

static bool contains(size_t numElements, void *elements[static numElements], void *value)
{
    for(size_t i = 0; i < numElements; ++i)
    {
        if(elements[i] == value)
            return true;
    }
    return false;
}

static void RectSelect(EdState *state, bool add)
{
    FVec2 min = fvec2_min(state->data.startDrag, state->data.endDrag);
    FVec2 max = fvec2_max(state->data.startDrag, state->data.endDrag);

    if(!add)
        state->data.numSelectedElements = 0;

    switch(state->data.selectionMode)
    {
    case MODE_VERTEX:
        {
            for(MapVertex *vertex = state->map.headVertex; vertex; vertex = vertex->next)
            {
                if(within(min, max, vertex->pos) && !contains(state->data.numSelectedElements, state->data.selectedElements, vertex))
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
                if(within(min, max, line->a->pos) && within(min, max, line->b->pos) && !contains(state->data.numSelectedElements, state->data.selectedElements, line))
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

                if(allPointsIn && !contains(state->data.numSelectedElements, state->data.selectedElements, sector))
                {
                    state->data.selectedElements[state->data.numSelectedElements++] = sector;
                }
            }
        }
        break;
    }
}

static void AddEditVertex(EdState *state, FVec2 v)
{
    size_t idx = state->data.editVertexBufferSize++;
    state->data.editVertexBuffer[idx] = v;
}

static void GotoLocation(EdState *state, real_t x, real_t y)
{
    state->data.viewPosition = (Vec2){ x - (state->gl.editorFramebufferWidth / 2.0f), x - (state->gl.editorFramebufferHeight / 2.0f) };
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
            GotoLocation(state, 0, 0);

        igSameLine(0, 16);
        if(igButton("Go To", (ImVec2){ 0, 0 }))
            igOpenPopup_Str("Go To", 0);

        if(igBeginPopupModal("Go To", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            static float coords[2] = { 0 };
            igInputFloat2("Coords", coords, NULL, 0);
            if(igButton("Goto", (ImVec2){ 0, 0 }))
            {
                GotoLocation(state, coords[0], coords[1]);
                igCloseCurrentPopup();
            }
            igSameLine(0, 4);
            if(igButton("Cancel", (ImVec2){ 0, 0 }))
            {
                igCloseCurrentPopup();
            }
            igEndPopup();
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

            real_t edX = relX, edSX = relX, edY = relY, edSY = relY;
            ScreenToEditorSpace(state, &edX, &edY);
            ScreenToEditorSpaceGrid(state, state->data.gridSize, &edSX, &edSY);

            fixed_t x1 = fixed_from_real(edX), y1 = fixed_from_real(edY);
            fixed_t x2 = fixed_from_real(edSX), y2 = fixed_from_real(edSY);

            bool shiftDown = igGetIO_Nil()->KeyShift;
            bool altDown = igGetIO_Nil()->KeyAlt;
            bool ctrlDown = igGetIO_Nil()->KeyCtrl;
            Map *map = &state->map;

            if(state->data.editState == ESTATE_ADDVERTEX)
            {
                if(altDown)
                {
                    MapLine *closestLine = EditGetClosestLine(map, (FVec2){ x1, y1 }, LINE_DIST + fixed_from_int(128));
                    if(closestLine)
                    {
                        FVec2 closestPoint = LineGetClosestPointGrid((line_t){ closestLine->a->pos, closestLine->b->pos }, (FVec2){ x1, y1 }, shiftDown ? state->data.altGridSize : state->data.gridSize);
                        x2 = closestPoint.x;
                        y2 = closestPoint.y;
                    }
                }
                else if(ctrlDown)
                {
                    MapVertex *closestVertex = EditGetClosestVertex(map, (FVec2){ .x = x1, .y = y1 }, VERTEX_DIST + fixed_from_int(64));
                    if(closestVertex)
                    {
                        x2 = closestVertex->pos.x;
                        y2 = closestVertex->pos.y;
                    }
                }
                else if(shiftDown)
                {
                    edSX = relX, edSY = relY;
                    ScreenToEditorSpaceGrid(state, state->data.altGridSize, &edSX, &edSY);
                    x2 = fixed_from_real(edSX);
                    y2 = fixed_from_real(edSY);
                }
            }

            if(hovored)
            {
#ifdef _DEBUG
                state->data.mx = fixed_to_real(x1);
                state->data.my = fixed_to_real(y1);
                state->data.mtx = fixed_to_real(x2);
                state->data.mty = fixed_to_real(y2);
#endif
                FVec2 mouseVertex = { x1, y1 };
                state->data.editVertexMouse = (FVec2){ .x = x2, .y = y2 };
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
                        state->data.isDragging = false;
                        RectSelect(state, shiftDown);
                    }
                }

                if(igIsMouseClicked_Bool(ImGuiMouseButton_Left, false) && !state->data.isDragging)
                {
                    FVec2 mouseVertexSnap = { x2, y2 };
                    if(state->data.editState == ESTATE_ADDVERTEX)
                    {
                        if(state->data.editVertexBufferSize == 0)
                        {
                            AddEditVertex(state, mouseVertexSnap);
                        }
                        else
                        {
                            FVec2 first = state->data.editVertexBuffer[0];
                            if(mouseVertexSnap.x == first.x && mouseVertexSnap.y == first.y && state->data.editVertexBufferSize >= 3)
                            {
                                // submit to edit
                                SubmitEditData(state, true);

                                state->data.editState = ESTATE_NORMAL;
                            }
                            else
                            {
                                FVec2 last = state->data.editVertexBuffer[state->data.editVertexBufferSize-1];
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

                if(igIsKeyPressed_Bool(ImGuiKey_F, false))
                {
                    if(state->data.editState == ESTATE_NORMAL && state->data.selectionMode == MODE_LINE)
                    {
                        //assert(false && "Not yet fully implemented");
                        for(size_t i = 0; i < state->data.numSelectedElements; ++i)
                        {
                            MapLine *line = state->data.selectedElements[i];
                            MapSector *front = line->frontSector, *back = line->backSector;
                            MapVertex *tmp = line->b;
                            line->b = line->a;
                            line->a = tmp;

                            line->frontSector = back;
                            line->backSector = front;
                        }
                    }
                }

                if(igIsKeyPressed_Bool(ImGuiKey_M, false))
                {
                    if(state->data.editState == ESTATE_NORMAL)
                    {
                        MapLine *line = EditGetClosestLine(map, (FVec2){ x1, y1 }, fixed_from_int(128));
                        if(line)
                        {
                            bool front = SideOfMapLine(line, (FVec2){ x1, y1 }) > 0;
                            if((front && line->frontSector == NULL) || (!front && line->backSector == NULL))
                            {
                                MapSector *sector = MakeMapSector(map, line, !front, DefaultSectorData());
                                if(!sector)
                                    LogError("Failed to make sector");
                            }
                            else
                            {
                                LogInfo("Sector already exists");
                            }
                        }
                    }
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
                        if(state->data.editVertexBufferSize > 2)
                        {
                            // submit edit data
                            SubmitEditData(state, true);
                            state->data.editVertexBufferSize = 0;
                            state->data.editState = ESTATE_NORMAL;
                        }
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
                state->data.viewPosition = (Vec2){ -state->gl.editorFramebufferWidth / 2.0f, -state->gl.editorFramebufferHeight / 2.0f };
            }
        }
        igEndChild();
    }
    igEnd();
}
