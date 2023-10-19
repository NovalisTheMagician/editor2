#include "../gwindows.h"

#include "../texture_collection.h"

static void SelectElement(struct EdState *state, void *element, int selectMode)
{
    state->data.numSelectedElements = 1;
    state->data.selectedElements[0] = element;
    state->data.selectionMode = selectMode;
}

static void VertexProperties(struct EdState *state)
{
    if(state->data.numSelectedElements == 1)
    {
        struct MapVertex *selectedVertex = state->data.selectedElements[0];
        igText("Index: %d", selectedVertex->idx);
        igText("RefCount: %d", selectedVertex->refCount);
    }
    else if(state->data.numSelectedElements > 1)
    {

    }
    else
    {
        igText("Index: -");
        igText("RefCount: -");
    }
}

static void LineProperties(struct EdState *state)
{
    if(state->data.numSelectedElements == 1)
    {
        struct MapLine *selectedLine = state->data.selectedElements[0];
        igText("Index: %d", selectedLine->idx);
        igText("RefCount: %d", selectedLine->refCount);
        igText("Vertex A: %d", selectedLine->a->idx); 
        igSameLine(0, 4);
        if(igButton("Select##a", (ImVec2){ 0, 0 }))
        {
            SelectElement(state, selectedLine->a, MODE_VERTEX);
        }
        igText("Vertex B: %d", selectedLine->b->idx);
        igSameLine(0, 4);
        if(igButton("Select##b", (ImVec2){ 0, 0 }))
        {
            SelectElement(state, selectedLine->b, MODE_VERTEX);
        }
        igText("Normal: %d", selectedLine->normal);
    }
    else if(state->data.numSelectedElements > 1)
    {

    }
    else
    {
        igText("Index: -");
        igText("RefCount: -");
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
                pstr_free(selectedSector->floorTex);
                selectedSector->floorTex = pstr_copy(selectedTexture->name);
            }
            else
            {
                pstr_free(selectedSector->ceilTex);
                selectedSector->ceilTex = pstr_copy(selectedTexture->name);
            }
        }
    }
    else if(state->data.numSelectedElements > 1)
    {
    }
    else
    {
        igText("Index: -");
    }
}

void PropertyWindow(bool *p_open, struct EdState *state)
{
    igSetNextWindowSize((ImVec2){ 300, 600 }, ImGuiCond_FirstUseEver);
    if(igBegin("Properties", p_open, 0))
    {
        switch(state->data.selectionMode)
        {
        case MODE_VERTEX: VertexProperties(state); break;
        case MODE_LINE: LineProperties(state); break;
        case MODE_SECTOR: SectorProperties(state); break;
        }
    }
    igEnd();
}
