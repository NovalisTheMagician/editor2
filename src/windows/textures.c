#include "../gwindows.h"

#include "cimgui.h"
#include "texture_load.h"

typedef struct IterateData
{
    EdState *state;
    Texture *selected;
    ImVec2 clientArea;
    int occupiedX;
} IterateData;

static void TextureIteration(Texture *texture, size_t idx, void *user)
{
    IterateData *data = user;

    ImVec2 size = { .x = texture->width, .y = texture->height };

    if(idx > 0 && data->occupiedX + size.x + 8 < data->clientArea.x)
        igSameLine(0, 2);
    else
        data->occupiedX = 0;

    if(igImageButton(texture->name, (void*)(intptr_t)texture->texture1, size, (ImVec2){ 0, 0 }, (ImVec2){ 1, 1 }, (ImVec4){ 1, 1, 1, 1 }, (ImVec4){ 1, 1, 1, 1 }))
    {
        data->selected = texture;
    }

    if(igBeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover))
    {
        igImage((void*)(intptr_t)texture->texture1, size, (ImVec2){ 0, 0 }, (ImVec2){ 1, 1 }, (ImVec4){ 1, 1, 1, 1 }, (ImVec4){ 1, 1, 1, 1 });
        igSetDragDropPayload("TextureDnD", &texture, sizeof texture, 0);
        igEndDragDropSource();
    }

    if (igIsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        igSetTooltip("Name: %s\nSize: %dx%d", texture->name, (int)size.x, (int)size.y);
    }

    data->occupiedX += size.x + 10;
}

void TexturesWindow(bool *p_open, EdState *state)
{
    if(igBegin("Texture Browser", p_open, ImGuiWindowFlags_MenuBar))
    {
        if(igBeginMenuBar())
        {
            if(igMenuItem_Bool("Refresh", "", false, true))
            {
                LoadTextures(state, true);
            }
            if(igMenuItem_Bool("Invalidate", "", false, true))
            {
                LoadTextures(state, false);
            }
            if(igMenuItem_Bool("Cancel Fetch", "", false, Async_IsRunningJob(&state->async)))
            {
                Async_AbortJob(&state->async);
            }
            igEndMenuBar();
        }

        igInputText("Filter", state->data.textureFilter, sizeof state->data.textureFilter, 0, NULL, NULL);
        igSameLine(0, 16);
        igText("%d Textures Found", tc_size(&state->textures));

        if(igBeginChild_ID(2002, (ImVec2){ 0, 0 }, true, ImGuiWindowFlags_AlwaysVerticalScrollbar))
        {
            ImVec2 clientArea;
            igGetContentRegionAvail(&clientArea);

            IterateData data = { .state = state, .clientArea = clientArea };

            if(state->data.textureFilter[0] == '\0')
                tc_iterate(&state->textures, TextureIteration, &data);
            else
                tc_iterate_filter(&state->textures, TextureIteration, state->data.textureFilter, &data);
        }
        igEndChild();
    }
    igEnd();
}
