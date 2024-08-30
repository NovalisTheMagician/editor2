#include "editor.h"

void ResizeRealtimeView(EdState *state, int width, int height)
{
    if(width == 0 || height == 0)
        return;

    if(state->gl.realtimeFramebufferWidth == width && state->gl.realtimeFramebufferHeight == height)
        return;

    if(state->gl.realtimeColorTexture > 0)
    {
        glDeleteTextures(2, (GLuint[]){ state->gl.realtimeColorTexture, state->gl.realtimeDepthTexture });
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &state->gl.realtimeColorTexture);
    glTextureStorage2D(state->gl.realtimeColorTexture, 1, GL_RGBA8, width, height);
    glTextureParameteri(state->gl.realtimeColorTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(state->gl.realtimeColorTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glCreateTextures(GL_TEXTURE_2D, 1, &state->gl.realtimeDepthTexture);
    glTextureStorage2D(state->gl.realtimeDepthTexture, 1, GL_DEPTH24_STENCIL8, width, height);
    glTextureParameteri(state->gl.realtimeDepthTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(state->gl.realtimeDepthTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glNamedFramebufferTexture(state->gl.realtimeFramebuffer, GL_COLOR_ATTACHMENT0, state->gl.realtimeColorTexture, 0);
    glNamedFramebufferTexture(state->gl.realtimeFramebuffer, GL_DEPTH_STENCIL_ATTACHMENT, state->gl.realtimeDepthTexture, 0);

    state->gl.realtimeFramebufferWidth = width;
    state->gl.realtimeFramebufferHeight = height;
}

void RenderRealtimeView(EdState *state)
{

}
