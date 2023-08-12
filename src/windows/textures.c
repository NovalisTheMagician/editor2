#include "../gwindows.h"

static void LoadTextures(struct TextureCollection *tc, struct Project *project, bool refresh)
{
    if(!refresh)
        tc_unload_all(tc);
}

struct Texture TexturesWindow(bool *p_open, struct EdState *state, bool popup)
{
    if(tc_size(&state->textures) == 0)
        LoadTextures(&state->textures, &state->project, false);

    if(igBegin("Texture Browser", p_open, ImGuiWindowFlags_MenuBar))
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


    }
    igEnd();

    return (struct Texture) { 0 };
}
