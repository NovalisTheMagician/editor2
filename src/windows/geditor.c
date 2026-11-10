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

#define EDITOR_ID 1000

#define DEFAULT_WHITE { 1, 1, 1, 1 }
#define LINE_DIST 10
#define VERTEX_DIST 5

static Vec2 ScreenToEditorSpace(const EdState *state, Vec2 pos)
{
    const real_t z = state->data.zoomLevel;
    pos.x = (pos.x + state->data.viewPosition.x) / z;
    pos.y = (pos.y + state->data.viewPosition.y) / z;
    return pos;
}

static Vec2 EditorToScreenSpace(const EdState *state, Vec2 pos)
{
    const real_t z = state->data.zoomLevel;
    pos.x = ((pos.x - state->data.viewPosition.x) * z);
    pos.y = ((pos.y - state->data.viewPosition.y) * z);
    return pos;
}

static Vec2 ScreenToEditorSpaceGrid(const EdState *state, int gridsize, Vec2 pos)
{
    pos = ScreenToEditorSpace(state, pos);
    const int offset = gridsize / 2;
    real_t xt = (int)(pos.x), yt = (int)(pos.y);
    xt += xt < 0 ? -offset : offset;
    pos.x = (int)xt / gridsize * gridsize;
    yt += yt < 0 ? -offset : offset;
    pos.y = (int)yt / gridsize * gridsize;
    return pos;
}

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

static bool within(Vec2 min, Vec2 max, Vec2 v)
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
    Vec2 min = vec2_min(state->data.startDrag, state->data.endDrag);
    Vec2 max = vec2_max(state->data.startDrag, state->data.endDrag);

    size_t count = 0;
    if(add)
        count += state->data.numSelectedElements;

    switch(state->data.selectionMode)
    {
    case MODE_VERTEX:
        {
            for(MapVertex *vertex = state->map.headVertex; vertex; vertex = vertex->next)
            {
                if(within(min, max, vertex->pos) && (!add || !contains(state->data.numSelectedElements, state->data.selectedElements, vertex)))
                {
                    state->data.selectedElements[count++] = vertex;
                }
            }
        }
        break;
    case MODE_LINE:
        {
            for(MapLine *line = state->map.headLine; line; line = line->next)
            {
                if(within(min, max, line->a->pos) && within(min, max, line->b->pos) && (!add || !contains(state->data.numSelectedElements, state->data.selectedElements, line)))
                {
                    state->data.selectedElements[count++] = line;
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

                if(allPointsIn && (!add || !contains(state->data.numSelectedElements, state->data.selectedElements, sector)))
                {
                    state->data.selectedElements[count++] = sector;
                }
            }
        }
        break;
    }
    if(add || (!add && count > 0))
        state->data.numSelectedElements = count;
}

static void AddEditVertex(EdState *state, Vec2 v)
{
    size_t idx = state->data.editVertexBufferSize++;
    state->data.editVertexBuffer[idx] = v;
}

static void MoveElements(EdState *state, Vec2 firstPosition)
{
    
    switch(state->data.selectionMode)
    {
    case MODE_VERTEX:
        break;
    case MODE_LINE:
        break;
    case MODE_SECTOR:
        break;
    }
}

static void GotoLocation(EdState *state, real_t x, real_t y)
{
    state->data.viewPosition = (Vec2){ x - (state->gl.editorFramebufferWidth / 2.0f), y - (state->gl.editorFramebufferHeight / 2.0f) };
}

static void HandleShortcuts(EdState *state)
{
    if(igShortcut_Nil(ImGuiMod_Ctrl | ImGuiKey_C, 0))
        EditCopy(state);
    if(igShortcut_Nil(ImGuiMod_Ctrl | ImGuiKey_V, 0))
        EditPaste(state);
    if(igShortcut_Nil(ImGuiMod_Ctrl | ImGuiKey_X, 0))
        EditCut(state);
    if(igShortcut_Nil(ImGuiMod_Ctrl | ImGuiKey_Z, 0))
        LogDebug("Undo!");
    if(igShortcut_Nil(ImGuiMod_Ctrl | ImGuiKey_Y, 0))
        LogDebug("Redo!");

    if(igShortcut_Nil(ImGuiKey_1, 0))
    {
        ChangeMode(state, MODE_VERTEX);
        //igSetWindowFocus_Str("Editor");
    }
    if(igShortcut_Nil(ImGuiKey_2, 0))
    {
        ChangeMode(state, MODE_LINE);
        //igSetWindowFocus_Str("Editor");
    }
    if(igShortcut_Nil(ImGuiKey_3, 0))
    {
        ChangeMode(state, MODE_SECTOR);
        //igSetWindowFocus_Str("Editor");
    }
}

static void HandleInput(EdState *state, Vec2 mousePos)
{
    Map *map = &state->map;
    if(igIsMouseClicked_Bool(ImGuiMouseButton_Middle, false))
    {
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
            MapLine *line = EditGetClosestLine(map, mousePos, 128);
            if(line)
            {
                bool front = SideOfMapLine(line, mousePos) > 0;
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

    if(igIsKeyPressed_Bool(ImGuiKey_G, false) && state->data.editState == ESTATE_NORMAL)
    {
        if(state->data.numSelectedElements > 0)
        {
            state->data.editState = ESTATE_MOVE_ELEMENT;
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

static void HandleDragSelect(EdState *state, Vec2 mousePos, bool additive)
{
    if(state->data.isDragging)
    {
        state->data.endDrag = mousePos;

        state->data.editVertexDrag[1].y = mousePos.y;
        state->data.editVertexDrag[2].x = mousePos.x;
    }
   
    if(igIsMouseDragging(ImGuiMouseButton_Left, 2) && state->data.editState != ESTATE_ADDVERTEX)
    {
        igSetWindowFocus_Nil();
        igResetMouseDragDelta(ImGuiMouseButton_Left);

        if(!state->data.isDragging)
        {
            state->data.isDragging = true;
            state->data.startDrag = mousePos;
            state->data.endDrag = mousePos;

            state->data.editVertexDrag[0] = mousePos; // start position
            state->data.editVertexDrag[1] = mousePos; // top right
            state->data.editVertexDrag[2] = mousePos; // bottom left
        }
    }
    else if(!igIsMouseDown_Nil(ImGuiMouseButton_Left))
    {
        if(state->data.isDragging)
        {
            state->data.isDragging = false;
            RectSelect(state, additive);
        }
    }
}

static void HandleDrag(EdState *state, bool focused)
{
    if(igIsMouseDragging(ImGuiMouseButton_Right, 2))
    {
        if(state->data.mouseDragCapture == 0 || state->data.mouseDragCapture == EDITOR_ID)
        {
            if(state->data.mouseDragCapture == 0 && !focused)
                return;
            else if(state->data.mouseDragCapture == 0 && focused)
            {
                state->data.mouseDragCapture = EDITOR_ID;
                igSetWindowFocus_Nil();
            }

            ImVec2 dragDelta;
            igGetMouseDragDelta(&dragDelta, ImGuiMouseButton_Right, 2);
            igResetMouseDragDelta(ImGuiMouseButton_Right);

            state->data.viewPosition.x -= dragDelta.x;
            state->data.viewPosition.y -= dragDelta.y;
        }
    }
    else if(state->data.mouseDragCapture == EDITOR_ID)
    {
        state->data.mouseDragCapture = 0;
    }
}

static void HandleZoom(EdState *state, Vec2 rawMousePos, Vec2 mousePos)
{
    ImGuiKeyData *wheelData = igGetKeyData_Key(ImGuiKey_MouseWheelY);
    if(wheelData->AnalogValue != 0)
    {
        igSetWindowFocus_Nil();
        state->data.zoomLevel += wheelData->AnalogValue * 0.05f;
        state->data.zoomLevel = clamp(MIN_ZOOM, MAX_ZOOM, state->data.zoomLevel);

        Vec2 mousePosAfter = ScreenToEditorSpace(state, rawMousePos);

        state->data.viewPosition.x += (mousePos.x - mousePosAfter.x) * state->data.zoomLevel;
        state->data.viewPosition.y += (mousePos.y - mousePosAfter.y) * state->data.zoomLevel;
    }
}

void EditorWindow(bool *p_open, EdState *state)
{
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar /*| ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize*/;
    if(state->map.dirty)
        flags |= ImGuiWindowFlags_UnsavedDocument;

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

        if(igBeginChild_ID(EDITOR_ID, (ImVec2){ 0, 0 }, false, ImGuiWindowFlags_NoMove))
        {
            ImVec2 clientArea;
            igGetContentRegionAvail(&clientArea);

            ImVec2 clientPos;
            igGetWindowPos(&clientPos);

            bool focused = igIsWindowHovered(0);

            ImVec2 mpos;
            igGetMousePos(&mpos);
            int relX = (int)mpos.x - (int)clientPos.x;
            int relY = (int)mpos.y - (int)clientPos.y;

            Vec2 mousePos = ScreenToEditorSpace(state, (Vec2){ relX, relY });
            Vec2 vertexPos = ScreenToEditorSpaceGrid(state, state->data.gridSize, (Vec2){ relX, relY });

            bool shiftDown = igGetIO_Nil()->KeyShift;
            bool altDown = igGetIO_Nil()->KeyAlt;
            bool ctrlDown = igGetIO_Nil()->KeyCtrl;
            Map *map = &state->map;

            if(state->data.editState == ESTATE_ADDVERTEX)
            {
                if(altDown)
                {
                    MapLine *closestLine = EditGetClosestLine(map, mousePos, LINE_DIST + 128);
                    if(closestLine)
                    {
                        vertexPos = LineGetClosestPointGrid((line_t){ closestLine->a->pos, closestLine->b->pos }, mousePos, shiftDown ? state->data.altGridSize : state->data.gridSize);
                    }
                }
                else if(ctrlDown)
                {
                    MapVertex *closestVertex = EditGetClosestVertex(map, mousePos, VERTEX_DIST + 64);
                    if(closestVertex)
                    {
                        vertexPos = closestVertex->pos;
                    }
                }
                else if(shiftDown)
                {
                    vertexPos = ScreenToEditorSpaceGrid(state, state->data.altGridSize, (Vec2){ relX, relY });
                }
            }
            
            HandleDrag(state, focused);
            if(focused)
            {
                HandleShortcuts(state);

                HandleDragSelect(state, mousePos, shiftDown);
                HandleZoom(state, (Vec2){ relX, relY }, mousePos);

#ifdef _DEBUG
                state->data.mx = mousePos.x;
                state->data.my = mousePos.y;
                state->data.mtx = vertexPos.x;
                state->data.mty = vertexPos.y;
#endif
                state->data.editVertexMouse = vertexPos;
                state->data.editDragMouse = mousePos;
                if(state->data.editState == ESTATE_NORMAL)
                {
                    switch(state->data.selectionMode)
                    {
                    case MODE_VERTEX: state->data.hoveredElement = EditGetClosestVertex(map, mousePos, VERTEX_DIST); break;
                    case MODE_LINE: state->data.hoveredElement = EditGetClosestLine(map, mousePos, LINE_DIST); break;
                    case MODE_SECTOR: state->data.hoveredElement = EditGetSector(map, mousePos); break;
                    }
                }

                if(igIsMouseClicked_Bool(ImGuiMouseButton_Left, false) && !state->data.isDragging)
                {
                    if(state->data.editState == ESTATE_ADDVERTEX)
                    {
                        if(state->data.editVertexBufferSize == 0)
                        {
                            AddEditVertex(state, vertexPos);
                        }
                        else
                        {
                            Vec2 first = state->data.editVertexBuffer[0];
                            if(vec2_eq(vertexPos, first) && state->data.editVertexBufferSize >= 3)
                            {
                                // submit to edit
                                SubmitEditData(state, true);

                                state->data.editState = ESTATE_NORMAL;
                            }
                            else
                            {
                                Vec2 last = state->data.editVertexBuffer[state->data.editVertexBufferSize-1];
                                if(!vec2_eq(vertexPos, last))
                                {
                                    AddEditVertex(state, vertexPos);
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
                        case MODE_VERTEX: selectedElement = EditGetClosestVertex(map, mousePos, VERTEX_DIST); break;
                        case MODE_LINE: selectedElement = EditGetClosestLine(map, mousePos, LINE_DIST); break;
                        case MODE_SECTOR: selectedElement = EditGetSector(map, mousePos); break;
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
                }

                HandleInput(state, mousePos);
            }

            ResizeEditorView(state, clientArea.x, clientArea.y);
            igImage((ImTextureRef){ ._TexID = state->gl.editorColorTexture }, clientArea, (ImVec2){ 0, 0 }, (ImVec2){ 1, 1 });

            static bool firstTime = true;
            // the clientarea check is a hack due to imgui setting the size to a very low value on startup if it's loaded from a saved layout
            // before it sets it correctly
            if(firstTime && clientArea.x > 10 && clientArea.y > 10)
            {
                firstTime = false;
                state->data.viewPosition = (Vec2){ -state->gl.editorFramebufferWidth / 2.0f, -state->gl.editorFramebufferHeight / 2.0f };
            }
        }
        igEndChild();
    }
    igEnd();
}
