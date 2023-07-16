#include "editor.h"

bool InitEditor(struct EdSettings *settings)
{
    glCreateFramebuffers(1, &settings->editorFramebuffer);

    return true;
}

void DestroyEditor(struct EdSettings *settings)
{
    glDeleteFramebuffers(1, &settings->editorFramebuffer);
}

void ResizeEditor(struct EdSettings *settings, int width, int height)
{
    if(settings->editorFramebufferWidth == width && settings->editorFramebufferHeight == height)
        return;

    if(settings->editorColorTexture > 0)
    {
        glDeleteTextures(1, &settings->editorColorTexture);
        glDeleteTextures(1, &settings->editorDepthTexture);
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &settings->editorColorTexture);
    glCreateTextures(GL_TEXTURE_2D, 1, &settings->editorDepthTexture);

    glTextureStorage2D(settings->editorColorTexture, 1, GL_RGBA8, width, height);
    glTextureStorage2D(settings->editorDepthTexture, 1, GL_DEPTH24_STENCIL8, width, height);

    glTextureParameteri(settings->editorColorTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(settings->editorColorTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTextureParameteri(settings->editorDepthTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(settings->editorDepthTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glNamedFramebufferTexture(settings->editorFramebuffer, GL_COLOR_ATTACHMENT0, settings->editorColorTexture, 0);
    glNamedFramebufferTexture(settings->editorFramebuffer, GL_DEPTH_STENCIL_ATTACHMENT, settings->editorDepthTexture, 0);

    settings->editorFramebufferWidth = width;
    settings->editorFramebufferHeight = height;
}

bool HandleInputEvents(const SDL_Event *e, struct EdSettings *settings, struct Map *map)
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

void RenderEditor(const struct EdSettings *settings, const struct Map *map)
{

}
