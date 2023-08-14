#include "../gwindows.h"

struct IterateData
{
    struct EdState *state;
    struct Texture *selected;
    ImVec2 clientArea;
    int occupiedX;
};

static void TextureIteration(struct Texture *texture, size_t idx, void *user)
{
    struct IterateData *data = user;

    ImVec2 size = { .x = texture->width, .y = texture->height };

    if(idx > 0 && data->occupiedX + size.x + 8 < data->clientArea.x)
        igSameLine(0, 2);
    else
        data->occupiedX = 0;

    if(igImageButton(pstr_tocstr(texture->name), (void*)(intptr_t)texture->texture1, size, (ImVec2){ 0, 0 }, (ImVec2){ 1, 1 }, (ImVec4){ 1, 1, 1, 1 }, (ImVec4){ 1, 1, 1, 1 }))
    {
        data->selected = texture;
    }

    if (igIsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        igSetTooltip("Name: %s\nSize: %dx%d", pstr_tocstr(texture->name), (int)size.x, (int)size.y);
    }

    data->occupiedX += size.x + 10;
}

struct Texture TexturesWindow(bool *p_open, struct EdState *state, bool popup)
{
    igSetNextWindowSize((ImVec2){ 800, 600 }, ImGuiCond_FirstUseEver);

    struct Texture selectedTexture = { 0 };

    ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar;

    bool open = popup ? igBeginPopup("textures_popup", flags) : igBegin("Texture Browser", p_open, flags);
    if(open)
    {
        if(igBeginMenuBar())
        {
            if(igMenuItem_Bool("Refresh", "", false, true)) 
            {
                LoadTextures(&state->textures, &state->project, true);
            }
            if(igMenuItem_Bool("Invalidate", "", false, true))
            {
                LoadTextures(&state->textures, &state->project, false);
            }
            igEndMenuBar();
        }

        igInputText_pstr("Filter", &state->data.textureFilter, 0, NULL, NULL);

        if(igBeginChild_ID(2002, (ImVec2){ 0, 0 }, true, ImGuiWindowFlags_AlwaysVerticalScrollbar))
        {
            ImVec2 clientArea;
            igGetContentRegionAvail(&clientArea);

            struct IterateData data = { .state = state, .clientArea = clientArea };

            if(state->data.textureFilter.data[0] == '\0')
                tc_iterate(&state->textures, TextureIteration, &data);
            else
                tc_iterate_filter(&state->textures, TextureIteration, state->data.textureFilter, &data);

            if(data.selected)
                selectedTexture = *data.selected;

            igEndChild();
        }
    }
    popup ? igEndPopup() : igEnd();

    return selectedTexture;
}
