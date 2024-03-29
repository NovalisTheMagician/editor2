#include "editor.h"

#include <tgmath.h>

#define SELECTION_CAPACITY 10000

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
    glCreateFramebuffers(1, &state->gl.editorFramebufferMS);
    glCreateFramebuffers(1, &state->gl.realtimeFramebuffer);

    static Color whiteColor = { 1, 1, 1, 1 };
    glCreateTextures(GL_TEXTURE_2D, 1, &state->gl.whiteTexture);
    glTextureStorage2D(state->gl.whiteTexture, 1, GL_RGBA8, 1, 1);
    glTextureSubImage2D(state->gl.whiteTexture, 0, 0, 0, 1, 1, GL_RGBA, GL_FLOAT, whiteColor);
    glTextureParameteri(state->gl.whiteTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(state->gl.whiteTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(state->gl.whiteTexture, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(state->gl.whiteTexture, GL_TEXTURE_WRAP_T, GL_REPEAT);

    state->data.gridSize = 32;
    state->data.zoomLevel = 1.0f;

    state->data.textureFilter = string_alloc(TEXTURE_FILTER_LEN);

    const GLbitfield 
	mapping_flags = GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT,
	storage_flags = GL_DYNAMIC_STORAGE_BIT | mapping_flags;
    size_t bufferSize = EDIT_VERTEXBUFFER_CAP * sizeof(struct VertexType);
    glCreateBuffers(1, &state->gl.editorEdit.buffer);
    glNamedBufferStorage(state->gl.editorEdit.buffer, bufferSize, NULL, storage_flags);
    state->gl.editorEdit.bufferMap = glMapNamedBufferRange(state->gl.editorEdit.buffer, 0, bufferSize, mapping_flags);

    state->data.autoScrollLogs = true;

    state->data.selectedElements = calloc(SELECTION_CAPACITY, sizeof *state->data.selectedElements);

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
    glDeleteFramebuffers(3, (GLuint[]){ state->gl.editorFramebuffer, state->gl.editorFramebufferMS, state->gl.realtimeFramebuffer });
    glDeleteTextures(5, (GLuint[])
                                { 
                                    state->gl.editorColorTexture,
                                    state->gl.editorColorTextureMS,
                                    state->gl.realtimeColorTexture, 
                                    state->gl.realtimeDepthTexture,
                                    state->gl.whiteTexture
                                });

    glDeleteProgram(state->gl.editorVertex.program);
    glDeleteBuffers(1, &state->gl.editorVertex.vertBuffer);
    glDeleteVertexArrays(1, &state->gl.editorVertex.vertFormat);

    glDeleteProgram(state->gl.editorLine.program);
    glDeleteBuffers(1, &state->gl.editorLine.vertBuffer);
    glDeleteVertexArrays(1, &state->gl.editorLine.vertFormat);

    glDeleteProgram(state->gl.editorSector.program);
    glDeleteBuffers(2, (GLuint[]){ state->gl.editorSector.vertBuffer, state->gl.editorSector.indBuffer });
    glDeleteVertexArrays(1, &state->gl.editorSector.vertFormat);

    glDeleteBuffers(1, &state->gl.editorEdit.buffer);

    free(state->data.selectedElements);

    string_free(state->data.textureFilter);
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
        glDeleteTextures(1, &state->gl.editorColorTextureMS);
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &state->gl.editorColorTexture);
    glTextureStorage2D(state->gl.editorColorTexture, 1, GL_RGBA8, width, height);
    glTextureParameteri(state->gl.editorColorTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(state->gl.editorColorTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &state->gl.editorColorTextureMS);
    glTextureStorage2DMultisample(state->gl.editorColorTextureMS, 8, GL_RGBA8, width, height, false);

    glNamedFramebufferTexture(state->gl.editorFramebuffer, GL_COLOR_ATTACHMENT0, state->gl.editorColorTexture, 0);
    glNamedFramebufferTexture(state->gl.editorFramebufferMS, GL_COLOR_ATTACHMENT0, state->gl.editorColorTextureMS, 0);

    state->gl.editorFramebufferWidth = width;
    state->gl.editorFramebufferHeight = height;

    vec2 horLine[] = {{ 0, 0 }, { width, 0 }};
    vec2 verLine[] = {{ 0, 0 }, { 0, height }};
    glNamedBufferSubData(state->gl.backgroundLinesBuffer, 0, 2 * sizeof(vec2), horLine);
    glNamedBufferSubData(state->gl.backgroundLinesBuffer, 2 * sizeof(vec2), 2 * sizeof(vec2), verLine);

    state->data.editorProjection = glms_ortho(0, width, 0, height, -1, 1);
}

void ChangeMode(struct EdState *state, enum SelectionMode mode)
{
    if(state->data.selectionMode != mode)
    {
        state->data.selectionMode = mode;
        state->data.numSelectedElements = 0;
    }
}

static void RenderBackground(const struct EdState *state)
{
    const float period = state->data.gridSize * state->data.zoomLevel;

    if(period >= 4 && state->settings.showGridLines)
    {
        const float offsetX = fmod(-state->data.viewPosition.x, period);
        const float offsetY = fmod(-state->data.viewPosition.y, period);

        const int vLines = (state->gl.editorFramebufferWidth / period) + 2;
        const int hLines = (state->gl.editorFramebufferHeight / period) + 2;

        const int hMajorIdx = floor(-state->data.viewPosition.y / period);
        const int vMajorIdx = floor(-state->data.viewPosition.x / period);

        glBindVertexArray(state->gl.editorBackProg.backVertexFormat);
        glUseProgram(state->gl.editorBackProg.hProgram);
        glUniform1f(state->gl.editorBackProg.hOffsetUniform, offsetY);
        glUniform1f(state->gl.editorBackProg.hPeriodUniform, period);
        glUniform4fv(state->gl.editorBackProg.hTintUniform, 1, state->settings.colors[COL_BACK_LINES]);
        glUniform4fv(state->gl.editorBackProg.hMajorTintUniform, 1, state->settings.colors[COL_BACK_MAJOR_LINES]);
        glUniformMatrix4fv(state->gl.editorBackProg.hVPUniform, 1, false, (float*)state->data.editorProjection.raw);
        if(state->settings.showMajorAxis)
            glUniform1i(state->gl.editorBackProg.hMajorIdxUniform, hMajorIdx);
        else 
            glUniform1i(state->gl.editorBackProg.hMajorIdxUniform, -1);

        glDrawArraysInstanced(GL_LINES, 0, 2, hLines);

        glUseProgram(state->gl.editorBackProg.vProgram);
        glUniform1f(state->gl.editorBackProg.vOffsetUniform, offsetX);
        glUniform1f(state->gl.editorBackProg.vPeriodUniform, period);
        glUniform4fv(state->gl.editorBackProg.vTintUniform, 1, state->settings.colors[COL_BACK_LINES]);
        glUniform4fv(state->gl.editorBackProg.vMajorTintUniform, 1, state->settings.colors[COL_BACK_MAJOR_LINES]);
        glUniformMatrix4fv(state->gl.editorBackProg.vVPUniform, 1, false, (float*)state->data.editorProjection.raw);
        if(state->settings.showMajorAxis)
            glUniform1i(state->gl.editorBackProg.vMajorIdxUniform, vMajorIdx);
        else
            glUniform1i(state->gl.editorBackProg.vMajorIdxUniform, -1);

        glDrawArraysInstanced(GL_LINES, 2, 2, vLines);
    }
    else if(state->settings.showMajorAxis)
    {
        const float offsetX = -state->data.viewPosition.x;
        const float offsetY = -state->data.viewPosition.y;

        const int hMajorIdx = 0;
        const int vMajorIdx = 0;

        glBindVertexArray(state->gl.editorBackProg.backVertexFormat);
        glUseProgram(state->gl.editorBackProg.hProgram);
        glUniform1f(state->gl.editorBackProg.hOffsetUniform, offsetY);
        glUniform1f(state->gl.editorBackProg.hPeriodUniform, 0);
        glUniform4fv(state->gl.editorBackProg.hTintUniform, 1, state->settings.colors[COL_BACK_LINES]);
        glUniform4fv(state->gl.editorBackProg.hMajorTintUniform, 1, state->settings.colors[COL_BACK_MAJOR_LINES]);
        glUniformMatrix4fv(state->gl.editorBackProg.hVPUniform, 1, false, (float*)state->data.editorProjection.raw);
        glUniform1i(state->gl.editorBackProg.hMajorIdxUniform, hMajorIdx);

        glDrawArraysInstanced(GL_LINES, 0, 2, 1);

        glUseProgram(state->gl.editorBackProg.vProgram);
        glUniform1f(state->gl.editorBackProg.vOffsetUniform, offsetX);
        glUniform1f(state->gl.editorBackProg.vPeriodUniform, 0);
        glUniform4fv(state->gl.editorBackProg.vTintUniform, 1, state->settings.colors[COL_BACK_LINES]);
        glUniform4fv(state->gl.editorBackProg.vMajorTintUniform, 1, state->settings.colors[COL_BACK_MAJOR_LINES]);
        glUniformMatrix4fv(state->gl.editorBackProg.vVPUniform, 1, false, (float*)state->data.editorProjection.raw);
        glUniform1i(state->gl.editorBackProg.vMajorIdxUniform, vMajorIdx);

        glDrawArraysInstanced(GL_LINES, 2, 2, 1);
    }
}

static inline bool includes(void * const *list, size_t size, const void *element)
{
    for(size_t i = 0; i < size; ++i)
    {
        if(list[i] == element) return true;
    }
    return false;
}

static void RenderVertices(const struct EdState *state, const mat4s viewProjMat)
{
    glPointSize(state->settings.vertexPointSize);
    glBindVertexArray(state->gl.editorVertex.vertFormat);
    glUseProgram(state->gl.editorVertex.program);
    glUniformMatrix4fv(state->gl.editorVertex.viewProjUniform, 1, false, (float*)viewProjMat.raw);

    for(const struct MapVertex *vertex = state->map.headVertex; vertex; vertex = vertex->next)
    {
        int colorIdx = COL_VERTEX;
        if(state->data.numSelectedElements > 0 && includes(state->data.selectedElements, state->data.numSelectedElements, vertex))
        {
            colorIdx = COL_VERTEX_SELECT;
        }
        else if(vertex == state->data.hoveredElement)
        {
            colorIdx = COL_VERTEX_HOVER;
        }

        glUniform4fv(state->gl.editorVertex.tintUniform, 1, state->settings.colors[colorIdx]);
        glDrawArrays(GL_POINTS, vertex->idx, 1);
    }
}

static void RenderLines(const struct EdState *state, const mat4s viewProjMat)
{
    glLineWidth(2);
    glBindVertexArray(state->gl.editorLine.vertFormat);
    glUseProgram(state->gl.editorLine.program);
    glUniformMatrix4fv(state->gl.editorLine.viewProjUniform, 1, false, (float*)viewProjMat.raw);

    for(const struct MapLine *line = state->map.headLine; line; line = line->next)
    {
        int colorIdx = COL_LINE;
        if(state->data.numSelectedElements > 0 && includes(state->data.selectedElements, state->data.numSelectedElements, line))
        {
            colorIdx = COL_LINE_SELECT;
        }
        else if(line == state->data.hoveredElement)
        {
            colorIdx = COL_LINE_HOVER;
        }
        /*
        else if(line->refCount > 1)
        {
            colorIdx = COL_LINE_INNER;
        }
        */
        glUniform4fv(state->gl.editorLine.tintUniform, 1, state->settings.colors[colorIdx]);
        glDrawArrays(GL_LINES, line->idx * 4, 4); // 2 verts per line segment | 2 for wall and 2 for wall normal
    }
    glLineWidth(1);
}

static void RenderSectors(const struct EdState *state, const mat4s viewProjMat)
{
    glBindVertexArray(state->gl.editorSector.vertFormat);
    glUseProgram(state->gl.editorSector.program);
    glUniformMatrix4fv(state->gl.editorSector.viewProjUniform, 1, false, (float*)viewProjMat.raw);
    glUniform1i(state->gl.editorSector.textureUniform, 0);
    if(!state->data.showSectorTextures)
        glBindTextureUnit(0, state->gl.whiteTexture);
    // handle real texture here

    for(const struct MapSector *sector = state->map.headSector; sector; sector = sector->next)
    {
        int colorIdx = COL_SECTOR;
        if(state->data.numSelectedElements > 0 && includes(state->data.selectedElements, state->data.numSelectedElements, sector))
        {
            colorIdx = COL_SECTOR_SELECT;
        }
        else if(sector == state->data.hoveredElement)
        {
            colorIdx = COL_SECTOR_HOVER;
        }

        glUniform4fv(state->gl.editorSector.tintUniform, 1, state->settings.colors[colorIdx]);
        const struct TriangleData data = sector->edData;
        //glDrawElementsBaseVertex(GL_TRIANGLES, data.indexLength, GL_UNSIGNED_INT, (void*)data.indexStart, data.vertexStart);
        glDrawElements(GL_TRIANGLES, data.indexLength, GL_UNSIGNED_INT, (void*)(data.indexStart * sizeof(Index_t)));
    }
}

static void RenderEditData(const struct EdState *state, const mat4s viewProjMat)
{
    if(state->data.editState == ESTATE_ADDVERTEX)
    {
        glPointSize(state->settings.vertexPointSize);
        glVertexArrayVertexBuffer(state->gl.editorVertex.vertFormat, 0, state->gl.editorEdit.buffer, 0, sizeof(struct VertexType));
        glBindVertexArray(state->gl.editorVertex.vertFormat);
        glUseProgram(state->gl.editorVertex.program);
        glUniformMatrix4fv(state->gl.editorVertex.viewProjUniform, 1, false, (float*)viewProjMat.raw);
        glUniform4fv(state->gl.editorVertex.tintUniform, 1, state->settings.colors[COL_ACTIVE_EDIT]);
        glDrawArrays(GL_POINTS, 4, state->data.editVertexBufferSize + 1);
        glVertexArrayVertexBuffer(state->gl.editorVertex.vertFormat, 0, state->gl.editorVertex.vertBuffer, 0, sizeof(struct VertexType));
    }

    if(state->data.editState == ESTATE_ADDVERTEX || state->data.isDragging)
    {
        glLineWidth(2);
        glVertexArrayVertexBuffer(state->gl.editorLine.vertFormat, 0, state->gl.editorEdit.buffer, 0, sizeof(struct VertexType));
        glBindVertexArray(state->gl.editorLine.vertFormat);
        glUseProgram(state->gl.editorLine.program);
        glUniformMatrix4fv(state->gl.editorLine.viewProjUniform, 1, false, (float*)viewProjMat.raw);
        glUniform4fv(state->gl.editorLine.tintUniform, 1, state->settings.colors[COL_ACTIVE_EDIT]);

        if(state->data.editState == ESTATE_ADDVERTEX)
            glDrawArrays(GL_LINE_STRIP, 4, state->data.editVertexBufferSize + 1);

        glLineWidth(1);
        if(state->data.isDragging)
            glDrawArrays(GL_LINE_LOOP, 0, 4);

        glVertexArrayVertexBuffer(state->gl.editorLine.vertFormat, 0, state->gl.editorLine.vertBuffer, 0, sizeof(struct VertexType));
    }
}

void RenderEditorView(struct EdState *state)
{
    mat4s viewMat = glms_translate_make((vec3s){{ -state->data.viewPosition.x, -state->data.viewPosition.y, 0 }});
    viewMat = glms_scale(viewMat, (vec3s){{ state->data.zoomLevel, state->data.zoomLevel, 1 }});
    mat4s viewProjMat = glms_mul(state->data.editorProjection, viewMat);

    RenderBackground(state);
    RenderSectors(state, viewProjMat);
    RenderLines(state, viewProjMat);
    RenderVertices(state, viewProjMat);
    RenderEditData(state, viewProjMat);
}
