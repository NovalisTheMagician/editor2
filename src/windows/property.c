#include "../gwindows.h"

#include "../texture_collection.h"
#include "cimgui.h"
#include "memory.h"

static void SelectElement(EdState *state, void *element, int selectMode)
{
    state->data.numSelectedElements = 1;
    state->data.selectedElements[0] = element;
    state->data.selectionMode = selectMode;
}

static void GotoPos(EdState *state, vec2s pos)
{
    state->data.viewPosition.x = -(state->gl.editorFramebufferWidth / 2.0f) + pos.x;
    state->data.viewPosition.y = -(state->gl.editorFramebufferHeight / 2.0f) + pos.y;
}

static void CenterVertex(EdState *state, MapVertex *vertex)
{
    GotoPos(state, vertex->pos);
}

static void CenterLine(EdState *state, MapLine *line)
{
    vec2s d = glms_vec2_sub(line->b->pos, line->a->pos);
    d = glms_vec2_scale(d, 0.5f);
    d = glms_vec2_add(d, line->a->pos);
    GotoPos(state, d);
}

static void CenterSector(EdState *state, MapSector *sector)
{
    BoundingBox bb = sector->bb;
    float w = bb.max.x - bb.min.x;
    float h = bb.max.y - bb.min.y;
    GotoPos(state, (vec2s){ .x = (w/2) + bb.min.x, .y = (h/2) + bb.min.y });
}

static void MapProperties(EdState *state)
{
    igSeparatorTextEx(0, "Map Properties", NULL, 0);
    igSliderInt("Texture Scale", &state->map.textureScale, 1, 10, "%dX", 0);
    igInputFloat("Gravity", &state->map.gravity, 0.01f, 0.1f, "%.2f", 0);
}

static void VertexProperties(EdState *state)
{
    igSeparatorTextEx(0, "Vertex Properties", NULL, 0);
    if(state->data.numSelectedElements == 1)
    {
        MapVertex *selectedVertex = state->data.selectedElements[0];
        igText("Index: %d", selectedVertex->idx);

        igText("Attached lines: %d", selectedVertex->numAttachedLines);
        igSeparatorEx(ImGuiSeparatorFlags_Horizontal, 2);
        for(size_t i = 0; i < selectedVertex->numAttachedLines; ++i)
        {
            MapLine *line = selectedVertex->attachedLines[i];
            igText("Line %lld:", line->idx);
            igSameLine(0, 4);
            char label[32] = {0};
            snprintf(label, sizeof label, "Select##id%zu", line->idx);
            if(igButton(label, (ImVec2){ 0, 0 }))
            {
                SelectElement(state, line, MODE_LINE);
                CenterLine(state, line);
                igSetWindowFocus_Str("Editor");
            }
        }
    }
    else if(state->data.numSelectedElements > 1)
    {

    }
}

static void LineProperties(EdState *state)
{
    igSeparatorTextEx(0, "Line Properties", NULL, 0);
    if(state->data.numSelectedElements == 1)
    {
        MapLine *selectedLine = state->data.selectedElements[0];
        igText("Index: %zu", selectedLine->idx);
        igText("Vertex A: %zu", selectedLine->a->idx);
        igSameLine(0, 4);
        if(igButton("Select##a", (ImVec2){ 0, 0 }))
        {
            SelectElement(state, selectedLine->a, MODE_VERTEX);
            CenterVertex(state, selectedLine->a);
            igSetWindowFocus_Str("Editor");
        }
        igText("Vertex B: %zu", selectedLine->b->idx);
        igSameLine(0, 4);
        if(igButton("Select##b", (ImVec2){ 0, 0 }))
        {
            SelectElement(state, selectedLine->b, MODE_VERTEX);
            CenterVertex(state, selectedLine->b);
            igSetWindowFocus_Str("Editor");
        }
        //igText("Normal: %d", selectedLine->normal);
        if(selectedLine->frontSector)
        {
            igText("Front Sector: %zu", selectedLine->frontSector->idx);
            igSameLine(0, 4);
            if(igButton("Select##fs", (ImVec2){ 0, 0 }))
            {
                SelectElement(state, selectedLine->frontSector, MODE_SECTOR);
                CenterSector(state, selectedLine->frontSector);
                igSetWindowFocus_Str("Editor");
            }
        }
        if(selectedLine->backSector)
        {
            igText("Back Sector: %zu", selectedLine->backSector->idx);
            igSameLine(0, 4);
            if(igButton("Select##bs", (ImVec2){ 0, 0 }))
            {
                SelectElement(state, selectedLine->backSector, MODE_SECTOR);
                CenterSector(state, selectedLine->backSector);
                igSetWindowFocus_Str("Editor");
            }
        }
    }
    else if(state->data.numSelectedElements > 1)
    {

    }
}

static void SectorProperties(EdState *state)
{
    igSeparatorTextEx(0, "Sector Properties", NULL, 0);
    if(state->data.numSelectedElements == 1)
    {
        MapSector *selectedSector = state->data.selectedElements[0];
        igText("Index: %d", selectedSector->idx);
        igInputInt("Floor Height", &selectedSector->data.floorHeight, 1, 10, 0);
        igInputInt("Ceiling Height", &selectedSector->data.ceilHeight, 1, 10, 0);

        Texture *floorTexture = tc_get(&state->textures, selectedSector->data.floorTex);
        igText("Floor");
        if(floorTexture)
        {
            if(igImageButton("floorTexture", (ImTextureID)(intptr_t)floorTexture->texture1, (ImVec2){ floorTexture->width, floorTexture->height }, (ImVec2){ 0, 0 }, (ImVec2){ 1, 1, }, (ImVec4){ 0, 0, 0, 0 }, (ImVec4){ 1, 1, 1, 1 }))
            {

            }
            if(igIsItemHovered(0) && igIsMouseReleased_Nil(ImGuiMouseButton_Right))
            {
                string_free(selectedSector->data.floorTex);
                selectedSector->data.floorTex = NULL;
            }
        }
        else
        {
            igButton("Select Texture##floor", (ImVec2){ 0, 0 });
        }

        if(igBeginDragDropTarget())
        {
            const ImGuiPayload *payload = igAcceptDragDropPayload("TextureDnD", 0);
            if(payload)
            {
                string_free(selectedSector->data.floorTex);
                Texture tex = *(Texture*)payload->Data;
                selectedSector->data.floorTex = string_copy(tex.name);
            }
            igEndDragDropTarget();
        }

        Texture *ceilTexture = tc_get(&state->textures, selectedSector->data.ceilTex);
        igText("Ceiling");
        if(ceilTexture)
        {
            if(igImageButton("ceilTexture", (ImTextureID)(intptr_t)ceilTexture->texture1, (ImVec2){ ceilTexture->width, ceilTexture->height }, (ImVec2){ 0, 0 }, (ImVec2){ 1, 1, }, (ImVec4){ 0, 0, 0, 0 }, (ImVec4){ 1, 1, 1, 1 }))
            {
                string_free(selectedSector->data.ceilTex);
                selectedSector->data.ceilTex = NULL;
            }
        }
        else
        {
            igButton("Select Texture##ceil", (ImVec2){ 0, 0 });
        }

        if(igBeginDragDropTarget())
        {
            const ImGuiPayload *payload = igAcceptDragDropPayload("TextureDnD", 0);
            if(payload)
            {
                string_free(selectedSector->data.ceilTex);
                Texture tex = *(Texture*)payload->Data;
                selectedSector->data.ceilTex = string_copy(tex.name);
            }
            igEndDragDropTarget();
        }
    }
    else if(state->data.numSelectedElements > 1)
    {
    }
}

void PropertyWindow(bool *p_open, EdState *state)
{
    igSetNextWindowSize((ImVec2){ 300, 600 }, ImGuiCond_FirstUseEver);
    if(igBegin("Properties", p_open, 0))
    {
        if(state->data.numSelectedElements == 0)
        {
            MapProperties(state);
        }
        else
        {
            switch(state->data.selectionMode)
            {
            case MODE_VERTEX: VertexProperties(state); break;
            case MODE_LINE: LineProperties(state); break;
            case MODE_SECTOR: SectorProperties(state); break;
            }
        }
    }
    igEnd();
}
