#include "editor.h"

#include <tgmath.h>

static void message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const* message, void const* user_param)
{
    const char *src_str = ({
        char *v = "";
		switch (source) {
		case GL_DEBUG_SOURCE_API: v = "API"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: v = "WINDOW SYSTEM"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: v = "SHADER COMPILER"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY: v = "THIRD PARTY"; break;
		case GL_DEBUG_SOURCE_APPLICATION: v = "APPLICATION"; break;
		case GL_DEBUG_SOURCE_OTHER: v = "OTHER"; break;
		}
        v;
	});

	const char *type_str = ({
        char *v = "";
		switch (type) {
		case GL_DEBUG_TYPE_ERROR: v = "ERROR"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: v = "DEPRECATED_BEHAVIOR"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: v = "UNDEFINED_BEHAVIOR"; break;
		case GL_DEBUG_TYPE_PORTABILITY: v = "PORTABILITY"; break;
		case GL_DEBUG_TYPE_PERFORMANCE: v = "PERFORMANCE"; break;
		case GL_DEBUG_TYPE_MARKER: v = "MARKER"; break;
		case GL_DEBUG_TYPE_OTHER: v = "OTHER"; break;
		}
        v;
	});

	const char *severity_str = ({
        char *v = "";
		switch (severity) {
		case GL_DEBUG_SEVERITY_NOTIFICATION: v = "NOTIFICATION"; break;
		case GL_DEBUG_SEVERITY_LOW: v = "LOW"; break;
		case GL_DEBUG_SEVERITY_MEDIUM: v = "MEDIUM"; break;
		case GL_DEBUG_SEVERITY_HIGH: v = "HIGH"; break;
		}
        v;
	});
    printf("%s, %s, %s, %u: %s\n", src_str, type_str, severity_str, id, message);
}

bool InitEditor(struct EdState *state)
{
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(message_callback, NULL);

    glCreateFramebuffers(1, &state->gl.editorFramebuffer);
    glCreateFramebuffers(1, &state->gl.realtimeFramebuffer);

    state->data.gridSize = 32;
    state->data.zoomLevel = 1.0f;

    if(!LoadShaders(state))
        return false;

    return true;
}

void DestroyEditor(struct EdState *state)
{
    glDeleteBuffers(1, &state->gl.backgroundLinesBuffer);
    glDeleteVertexArrays(1, &state->gl.editorBackProg.backVertexFormat);
    glDeleteProgram(state->gl.editorBackProg.hProgram);
    glDeleteProgram(state->gl.editorBackProg.vProgram);
    glDeleteFramebuffers(2, (GLuint[]){ state->gl.editorFramebuffer, state->gl.realtimeFramebuffer });
    glDeleteTextures(3, (GLuint[])
                                { 
                                    state->gl.editorColorTexture,
                                    state->gl.realtimeColorTexture, 
                                    state->gl.realtimeDepthTexture
                                });

    glDeleteProgram(state->gl.editorVertex.program);
    glDeleteBuffers(1, &state->gl.editorVertex.vertBuffer);
    glDeleteVertexArrays(1, &state->gl.editorVertex.vertFormat);
}

void ResizeEditorView(struct EdState *state, int width, int height)
{
    if(width == 0 || height == 0)
        return;

    if(state->gl.editorFramebufferWidth == width && state->gl.editorFramebufferHeight == height)
        return;

    if(state->gl.editorColorTexture > 0)
    {
        glDeleteTextures(1, &state->gl.editorColorTexture);
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &state->gl.editorColorTexture);
    glTextureStorage2D(state->gl.editorColorTexture, 1, GL_RGBA8, width, height);
    glTextureParameteri(state->gl.editorColorTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(state->gl.editorColorTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glNamedFramebufferTexture(state->gl.editorFramebuffer, GL_COLOR_ATTACHMENT0, state->gl.editorColorTexture, 0);

    state->gl.editorFramebufferWidth = width;
    state->gl.editorFramebufferHeight = height;

    vec2 horLine[] = {{ 0, 0 }, { width, 0 }};
    vec2 verLine[] = {{ 0, 0 }, { 0, height }};
    glNamedBufferSubData(state->gl.backgroundLinesBuffer, 0, 2 * sizeof(vec2), horLine);
    glNamedBufferSubData(state->gl.backgroundLinesBuffer, 2 * sizeof(vec2), 2 * sizeof(vec2), verLine);

    glm_ortho(0, width, 0, height, -1, 1, state->data.editorProjection);
}

static void RenderBackground(const struct EdState *state)
{
    float offsetX = fmod(-state->data.viewPosition.x, (float)state->data.gridSize);
    float offsetY = fmod(-state->data.viewPosition.y, (float)state->data.gridSize);

    float period = state->data.gridSize;

    int vLines = (state->gl.editorFramebufferWidth / period) + 2;
    int hLines = (state->gl.editorFramebufferHeight / period) + 2;

    int hMajorIdx = -state->data.viewPosition.y / period;
    int vMajorIdx = -state->data.viewPosition.x / period;

    glBindVertexArray(state->gl.editorBackProg.backVertexFormat);
    glUseProgram(state->gl.editorBackProg.hProgram);
    glUniform1f(state->gl.editorBackProg.hOffsetUniform, offsetY);
    glUniform1f(state->gl.editorBackProg.hPeriodUniform, period);
    glUniform4fv(state->gl.editorBackProg.hTintUniform, 1, state->settings.colors[COL_BACK_LINES]);
    glUniform4fv(state->gl.editorBackProg.hMajorTintUniform, 1, state->settings.colors[COL_BACK_MAJOR_LINES]);
    glUniformMatrix4fv(state->gl.editorBackProg.hVPUniform, 1, false, (float*)state->data.editorProjection);
    glUniform1i(state->gl.editorBackProg.hMajorIdxUniform, hMajorIdx);

    glDrawArraysInstanced(GL_LINES, 0, 2, hLines);

    glUseProgram(state->gl.editorBackProg.vProgram);
    glUniform1f(state->gl.editorBackProg.vOffsetUniform, offsetX);
    glUniform1f(state->gl.editorBackProg.vPeriodUniform, period);
    glUniform4fv(state->gl.editorBackProg.vTintUniform, 1, state->settings.colors[COL_BACK_LINES]);
    glUniform4fv(state->gl.editorBackProg.vMajorTintUniform, 1, state->settings.colors[COL_BACK_MAJOR_LINES]);
    glUniformMatrix4fv(state->gl.editorBackProg.vVPUniform, 1, false, (float*)state->data.editorProjection);
    glUniform1i(state->gl.editorBackProg.vMajorIdxUniform, vMajorIdx);

    glDrawArraysInstanced(GL_LINES, 2, 2, vLines);
}

static void RenderVertices(const struct EdState *state, const mat4 viewProjMat)
{
    glPointSize(8);
    glBindVertexArray(state->gl.editorVertex.vertFormat);
    glUseProgram(state->gl.editorVertex.program);
    glUniformMatrix4fv(state->gl.editorVertex.viewProjUniform, 1, false, (float*)viewProjMat);
    glUniform4fv(state->gl.editorVertex.tintUniform, 1, state->settings.colors[COL_VERTEX]);

    glDrawArrays(GL_POINTS, 0, state->map.numVertices);
}

void RenderEditorView(struct EdState *state)
{
    mat4 viewProjMat, viewMat;
    glm_translate_make(viewMat, (vec3){ -state->data.viewPosition.x, -state->data.viewPosition.y, 0 });
    glm_mul(state->data.editorProjection, viewMat, viewProjMat);

    RenderBackground(state);
    RenderVertices(state, viewProjMat);
}
