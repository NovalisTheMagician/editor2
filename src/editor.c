#include "editor.h"

bool InitEditor(struct EdState *state)
{
    state->showToolbar = true;
    return true;
}

void DestroyEditor(struct EdState *state)
{
}

void ResizeEditor(struct EdState *state, int width, int height)
{
    
}

bool HandleInputEvents(const SDL_Event *e, struct EdState *state, struct Map *map)
{
    switch(e->type)
    {
    case SDL_KEYDOWN:
        printf("%s down\n", SDL_GetKeyName(e->key.keysym.sym));
        break;
    case SDL_KEYUP:
        printf("%s up\n", SDL_GetKeyName(e->key.keysym.sym));
        break;
    }
    return false;
}

void RenderEditor(const struct EdState *state, const struct Map *map)
{

}
