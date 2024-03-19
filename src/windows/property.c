#include "../gwindows.h"

#include "../texture_collection.h"

static void SelectElement(struct EdState *state, void *element, int selectMode)
{
    state->data.numSelectedElements = 1;
    state->data.selectedElements[0] = element;
    state->data.selectionMode = selectMode;
}

static void GotoPos(struct EdState *state, vec2s pos)
{
    state->data.viewPosition.x = -(state->gl.editorFramebufferWidth / 2) + pos.x;
    state->data.viewPosition.y = -(state->gl.editorFramebufferHeight / 2) + pos.y;
}

static void CenterVertex(struct EdState *state, struct MapVertex *vertex)
{
    GotoPos(state, vertex->pos);
}

static void CenterLine(struct EdState *state, struct MapLine *line)
{

}

static void CenterSector(struct EdState *state, struct MapSector *sector)
{
    struct BoundingBox bb = sector->bb;
    float w = bb.max.x - bb.min.x;
    float h = bb.max.y - bb.min.y;
    GotoPos(state, (vec2s){ .x = (w/2) + bb.min.x, .y = (h/2) + bb.min.y });
}

static void MapProperties(struct EdState *state)
{

}

static void VertexProperties(struct EdState *state)
{
    if(state->data.numSelectedElements == 1)
    {
        struct MapVertex *selectedVertex = state->data.selectedElements[0];
        igText("Index: %d", selectedVertex->idx);
    }
    else if(state->data.numSelectedElements > 1)
    {

    }
}

static void LineProperties(struct EdState *state)
{
    if(state->data.numSelectedElements == 1)
    {
        struct MapLine *selectedLine = state->data.selectedElements[0];
        igText("Index: %d", selectedLine->idx);
        igText("Vertex A: %d", selectedLine->a->idx);
        igSameLine(0, 4);
        if(igButton("Select##a", (ImVec2){ 0, 0 }))
        {
            SelectElement(state, selectedLine->a, MODE_VERTEX);
            CenterVertex(state, selectedLine->a);
            igSetWindowFocus_Str("Editor");
        }
        igText("Vertex B: %d", selectedLine->b->idx);
        igSameLine(0, 4);
        if(igButton("Select##b", (ImVec2){ 0, 0 }))
        {
            SelectElement(state, selectedLine->b, MODE_VERTEX);
            CenterVertex(state, selectedLine->b);
            igSetWindowFocus_Str("Editor");
        }
        //igText("Normal: %d", selectedLine->normal);
        igText("Front Sector: %d", selectedLine->frontSector ? selectedLine->frontSector->idx : -1);
        if(selectedLine->frontSector)
        {
            igSameLine(0, 4);
            if(igButton("Select##fs", (ImVec2){ 0, 0 }))
            {
                SelectElement(state, selectedLine->frontSector, MODE_SECTOR);
                CenterSector(state, selectedLine->frontSector);
                igSetWindowFocus_Str("Editor");
            }
        }
        igText("Back Sector: %d", selectedLine->backSector ? selectedLine->backSector->idx : -1);
        if(selectedLine->backSector)
        {
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

static void SectorProperties(struct EdState *state)
{
    if(state->data.numSelectedElements == 1)
    {
        struct MapSector *selectedSector = state->data.selectedElements[0];
        igText("Index: %d", selectedSector->idx);
        igInputInt("Floor Height", &selectedSector->floorHeight, 1, 10, 0);
        igInputInt("Ceiling Height", &selectedSector->ceilHeight, 1, 10, 0);

        /*
        static int textureToSet = 0;
        struct Texture *floorTexture = tc_get(&state->textures, selectedSector->floorTex);
        igText("Floor");
        if(floorTexture)
        {
            if(igImageButton("floorTexture", (ImTextureID)(intptr_t)floorTexture->texture1, (ImVec2){ floorTexture->width, floorTexture->height }, (ImVec2){ 0, 0 }, (ImVec2){ 1, 1, }, (ImVec4){ 0, 0, 0, 0 }, (ImVec4){ 1, 1, 1, 1 }))
            {
                igOpenPopup_Str("Texture Browser##textures_popup", 0);
                textureToSet = 0;
            }
        }
        else
        {
            if(igButton("Select Texture##floor", (ImVec2){ 0, 0 }))
            {
                igOpenPopup_Str("Texture Browser##textures_popup", 0);
                textureToSet = 0;
            }
        }
        struct Texture *ceilTexture = tc_get(&state->textures, selectedSector->ceilTex);
        igText("Ceiling");
        if(ceilTexture)
        {
            if(igImageButton("ceilTexture", (ImTextureID)(intptr_t)ceilTexture->texture1, (ImVec2){ ceilTexture->width, ceilTexture->height }, (ImVec2){ 0, 0 }, (ImVec2){ 1, 1, }, (ImVec4){ 0, 0, 0, 0 }, (ImVec4){ 1, 1, 1, 1 }))
            {
                igOpenPopup_Str("Texture Browser##textures_popup", 0);
                textureToSet = 1;
            }
        }
        else
        {
            if(igButton("Select Texture##ceil", (ImVec2){ 0, 0 }))
            {
                igOpenPopup_Str("Texture Browser##textures_popup", 0);
                textureToSet = 1;
            }
        }

        struct Texture *selectedTexture = TexturesWindow(NULL, state, true);
        if(selectedTexture)
        {
            if(textureToSet == 0)
            {
                string_free(selectedSector->floorTex);
                selectedSector->floorTex = string_copy(selectedTexture->name);
            }
            else
            {
                string_free(selectedSector->ceilTex);
                selectedSector->ceilTex = string_copy(selectedTexture->name);
            }
        }
        */
    }
    else if(state->data.numSelectedElements > 1)
    {
    }
}

void PropertyWindow(bool *p_open, struct EdState *state)
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
