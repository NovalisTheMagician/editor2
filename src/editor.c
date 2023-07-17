#include "editor.h"

bool InitEditor(struct EdState *state)
{
    glCreateFramebuffers(1, &state->editorFramebuffer);
    return true;
}

void DestroyEditor(struct EdState *state)
{
    glDeleteFramebuffers(1, &state->editorFramebuffer);
    glDeleteTextures(2, (GLuint[]){ state->editorColorTexture, state->editorDepthTexture });
}

void ResizeEditor(struct EdState *state, int width, int height)
{
    if(width == 0 || height == 0)
        return;

    if(state->editorFramebufferWidth == width && state->editorFramebufferHeight == height)
        return;

    if(state->editorColorTexture > 0)
    {
        glDeleteTextures(2, (GLuint[]){ state->editorColorTexture, state->editorDepthTexture });
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &state->editorColorTexture);
    glCreateTextures(GL_TEXTURE_2D, 1, &state->editorDepthTexture);

    glTextureStorage2D(state->editorColorTexture, 1, GL_RGBA8, width, height);
    glTextureStorage2D(state->editorDepthTexture, 1, GL_DEPTH24_STENCIL8, width, height);

    glTextureParameteri(state->editorColorTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(state->editorColorTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTextureParameteri(state->editorDepthTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(state->editorDepthTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glNamedFramebufferTexture(state->editorFramebuffer, GL_COLOR_ATTACHMENT0, state->editorColorTexture, 0);
    glNamedFramebufferTexture(state->editorFramebuffer, GL_DEPTH_STENCIL_ATTACHMENT, state->editorDepthTexture, 0);

    state->editorFramebufferWidth = width;
    state->editorFramebufferHeight = height;
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
