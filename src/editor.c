#include "editor.h"

bool InitEditor(struct EdState *state)
{
    glCreateFramebuffers(1, &state->gl.editorFramebuffer);
    return true;
}

void DestroyEditor(struct EdState *state)
{
    glDeleteFramebuffers(1, &state->gl.editorFramebuffer);
}

void ResizeEditor(struct EdState *state, int width, int height)
{
    if(width == 0 || height == 0)
        return;

    if(state->gl.editorFramebufferWidth == width && state->gl.editorFramebufferHeight == height)
        return;

    if(state->gl.editorColorTexture > 0)
    {
        glDeleteTextures(1, &state->gl.editorColorTexture);
        glDeleteTextures(1, &state->gl.editorDepthTexture);
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &state->gl.editorColorTexture);
    glCreateTextures(GL_TEXTURE_2D, 1, &state->gl.editorDepthTexture);

    glTextureStorage2D(state->gl.editorColorTexture, 1, GL_RGBA8, width, height);
    glTextureStorage2D(state->gl.editorDepthTexture, 1, GL_DEPTH24_STENCIL8, width, height);

    glTextureParameteri(state->gl.editorColorTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(state->gl.editorColorTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTextureParameteri(state->gl.editorDepthTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(state->gl.editorDepthTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glNamedFramebufferTexture(state->gl.editorFramebuffer, GL_COLOR_ATTACHMENT0, state->gl.editorColorTexture, 0);
    glNamedFramebufferTexture(state->gl.editorFramebuffer, GL_DEPTH_STENCIL_ATTACHMENT, state->gl.editorDepthTexture, 0);

    state->gl.editorFramebufferWidth = width;
    state->gl.editorFramebufferHeight = height;
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
