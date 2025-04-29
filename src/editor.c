#include "editor.h"

#include <string.h>
#include <tgmath.h>
#include <stb/stb_image.h>

#include "logging.h"
#include "map.h"
#include "utils.h"
#include "texture_collection.h"
#include "resources/resources.h"
#include "utils/debug.h"
#include "vertex_types.h"
#include "memory.h" // IWYU pragma: keep

#define SELECTION_CAPACITY 10000
#define BUFFER_SIZE (1<<20)
#define TEXTURE_SET_SIZE 8192
#define WHITE_TEXTURE (TEXTURE_SET_SIZE - 1)

static void message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei, GLchar const* message, void const*)
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

bool InitEditor(EdState *state, char *error, size_t errorSize)
{
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
    glDebugMessageCallback(message_callback, NULL);

    if(!LoadShaders(state, error, errorSize))
        return false;

    int w, h, c;
    uint8_t *pixels = stbi_load_from_memory(gDefaultTextureData, gDefaultTextureSize, &w, &h, &c, 4);
#ifdef _DEBUG
    debug_insertAddress(pixels, __FILE__, __LINE__);
#endif
    if(!pixels)
    {
        snprintf(error, errorSize, "failed to load the default texture");
        return false;
    }
    size_t numMipLevels = log2(max(w, h)) + 1;
    glCreateTextures(GL_TEXTURE_2D, 1, &state->defaultTextures.missingTexture);
    glTextureStorage2D(state->defaultTextures.missingTexture, numMipLevels, GL_RGBA8, w, h);
    glTextureSubImage2D(state->defaultTextures.missingTexture, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glGenerateTextureMipmap(state->defaultTextures.missingTexture);
    glTextureParameteri(state->defaultTextures.missingTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTextureParameteri(state->defaultTextures.missingTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(state->defaultTextures.missingTexture, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(state->defaultTextures.missingTexture, GL_TEXTURE_WRAP_T, GL_REPEAT);
    state->defaultTextures.missingTextureWidth = state->defaultTextures.missingTextureHeight = 64;
    free(pixels);

    glCreateFramebuffers(1, &state->gl.editorFramebuffer);
    glCreateFramebuffers(1, &state->gl.editorFramebufferMS);
    glCreateFramebuffers(1, &state->gl.realtimeFramebuffer);

    static Color whiteColor = { .r = 1, .g = 1, .b = 1, .a = 1 };
    glCreateTextures(GL_TEXTURE_2D, 1, &state->gl.whiteTexture);
    glTextureStorage2D(state->gl.whiteTexture, 1, GL_RGBA8, 1, 1);
    glTextureSubImage2D(state->gl.whiteTexture, 0, 0, 0, 1, 1, GL_RGBA, GL_FLOAT, whiteColor.raw);
    glTextureParameteri(state->gl.whiteTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(state->gl.whiteTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(state->gl.whiteTexture, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(state->gl.whiteTexture, GL_TEXTURE_WRAP_T, GL_REPEAT);

    state->data.gridSize = 32;
    state->data.zoomLevel = 1.0f;

    const GLbitfield
	mapping_flags = GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT,
	storage_flags = GL_DYNAMIC_STORAGE_BIT | mapping_flags;
    size_t bufferSize = BUFFER_SIZE * sizeof(EditorVertexType);
    glCreateBuffers(1, &state->gl.editorVertexBuffer);
    glNamedBufferStorage(state->gl.editorVertexBuffer, bufferSize, NULL, storage_flags);
    state->gl.editorVertexMap = glMapNamedBufferRange(state->gl.editorVertexBuffer, 0, bufferSize, mapping_flags);

    size_t iBufferSize = BUFFER_SIZE * sizeof(Index_t);
    glCreateBuffers(1, &state->gl.editorIndexBuffer);
    glNamedBufferStorage(state->gl.editorIndexBuffer, iBufferSize, NULL, storage_flags);
    state->gl.editorIndexMap = glMapNamedBufferRange(state->gl.editorIndexBuffer, 0, iBufferSize, mapping_flags);

    state->gl.editorMaxBufferCount = BUFFER_SIZE / NUM_BUFFERS;

    glCreateVertexArrays(1, &state->gl.editorVertexFormat);
    glEnableVertexArrayAttrib(state->gl.editorVertexFormat, 0);
    glEnableVertexArrayAttrib(state->gl.editorVertexFormat, 1);
    glEnableVertexArrayAttrib(state->gl.editorVertexFormat, 2);
    glVertexArrayAttribFormat(state->gl.editorVertexFormat, 0, 2, GL_FLOAT, GL_FALSE, offsetof(EditorVertexType, position));
    glVertexArrayAttribFormat(state->gl.editorVertexFormat, 1, 4, GL_FLOAT, GL_FALSE, offsetof(EditorVertexType, color));
    glVertexArrayAttribFormat(state->gl.editorVertexFormat, 2, 2, GL_FLOAT, GL_FALSE, offsetof(EditorVertexType, texCoord));
    glVertexArrayAttribBinding(state->gl.editorVertexFormat, 0, 0);
    glVertexArrayAttribBinding(state->gl.editorVertexFormat, 1, 0);
    glVertexArrayAttribBinding(state->gl.editorVertexFormat, 2, 0);
    glVertexArrayVertexBuffer(state->gl.editorVertexFormat, 0, state->gl.editorVertexBuffer, 0, sizeof(EditorVertexType));
    glVertexArrayElementBuffer(state->gl.editorVertexFormat, state->gl.editorIndexBuffer);

    glCreateVertexArrays(1, &state->gl.realtimeVertexFormat);
    glEnableVertexArrayAttrib(state->gl.realtimeVertexFormat, 0);
    glEnableVertexArrayAttrib(state->gl.realtimeVertexFormat, 1);
    glEnableVertexArrayAttrib(state->gl.realtimeVertexFormat, 2);
    glVertexArrayAttribFormat(state->gl.realtimeVertexFormat, 0, 3, GL_FLOAT, GL_FALSE, offsetof(RealtimeVertexType, position));
    glVertexArrayAttribFormat(state->gl.realtimeVertexFormat, 1, 4, GL_FLOAT, GL_FALSE, offsetof(RealtimeVertexType, color));
    glVertexArrayAttribFormat(state->gl.realtimeVertexFormat, 2, 2, GL_FLOAT, GL_FALSE, offsetof(RealtimeVertexType, texCoord));
    glVertexArrayAttribBinding(state->gl.realtimeVertexFormat, 0, 0);
    glVertexArrayAttribBinding(state->gl.realtimeVertexFormat, 1, 0);
    glVertexArrayAttribBinding(state->gl.realtimeVertexFormat, 2, 0);

    glCreateBuffers(1, &state->gl.editorShaderDataBuffer);
    glNamedBufferStorage(state->gl.editorShaderDataBuffer, sizeof(EditorShaderData), NULL, GL_DYNAMIC_STORAGE_BIT);

    state->data.autoScrollLogs = true;

    state->data.selectedElements = calloc(SELECTION_CAPACITY, sizeof *state->data.selectedElements);

    return true;
}

void DestroyEditor(EdState *state)
{
    GLuint framebuffers[] = { state->gl.editorFramebuffer, state->gl.editorFramebufferMS, state->gl.realtimeFramebuffer };
    glDeleteFramebuffers(COUNT_OF(framebuffers), framebuffers);
    GLuint textures[] = { state->gl.editorColorTexture, state->gl.editorColorTextureMS, state->gl.realtimeColorTexture, state->gl.realtimeDepthTexture, state->gl.whiteTexture, state->defaultTextures.missingTexture };
    glDeleteTextures(COUNT_OF(textures), textures);
    GLuint buffer[] = { state->gl.editorVertexBuffer, state->gl.editorIndexBuffer, state->gl.editorShaderDataBuffer, state->gl.backgroundLinesBuffer };
    glDeleteBuffers(COUNT_OF(buffer), buffer);
    GLuint formats[] = { state->gl.editorBackProg.backVertexFormat, state->gl.editorVertexFormat, state->gl.realtimeVertexFormat };
    glDeleteVertexArrays(COUNT_OF(formats), formats);

    for(size_t i = 0; i < NUM_BUFFERS; ++i)
        if(state->gl.editorBufferFence[i] != NULL)
            glDeleteSync(state->gl.editorBufferFence[i]);

    glDeleteProgram(state->gl.editorBackProg.hProgram);
    glDeleteProgram(state->gl.editorBackProg.vProgram);
    glDeleteProgram(state->gl.editorVertex.program);
    glDeleteProgram(state->gl.editorLine.program);
    glDeleteProgram(state->gl.editorSector.program);
    glDeleteProgram(state->gl.realtimeProgram.program);

    free(state->data.selectedElements);
}

void ResizeEditorView(EdState *state, int width, int height)
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

void ChangeMode(EdState *state, enum SelectionMode mode)
{
    if(state->data.selectionMode != (int)mode)
    {
        state->data.selectionMode = mode;
        state->data.numSelectedElements = 0;
    }
}

static void RenderBackground(const EdState *state)
{
    const float period = state->data.gridSize * state->data.zoomLevel;

    const float offsetX = fmod(-state->data.viewPosition.x, period);
    const float offsetY = fmod(-state->data.viewPosition.y, period);

    const int vLines = (state->gl.editorFramebufferWidth / period) + 2;
    const int hLines = (state->gl.editorFramebufferHeight / period) + 2;

    const int hMajorIdx = floor(-state->data.viewPosition.y / period);
    const int vMajorIdx = floor(-state->data.viewPosition.x / period);

    BackgroundShaderData shaderData =
    {
        .hOffset = offsetY,
        .period = period,
        .vOffset = offsetX,
        .tint = state->settings.colors[COL_BACK_LINES],
        .majorTint = state->settings.colors[COL_BACK_MAJOR_LINES],
        .majorIndex = -1,
        .viewProj = state->data.editorProjection
    };

    glBindVertexArray(state->gl.editorBackProg.backVertexFormat);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, state->gl.editorBackProg.shaderDataBuffer);
    if(period >= 4 && state->settings.showGridLines)
    {
        glUseProgram(state->gl.editorBackProg.hProgram);
        if(state->settings.showMajorAxis)
            shaderData.majorIndex = hMajorIdx;
        glNamedBufferSubData(state->gl.editorBackProg.shaderDataBuffer, 0, sizeof shaderData, &shaderData);
        glDrawArraysInstanced(GL_LINES, 0, 2, hLines);

        glUseProgram(state->gl.editorBackProg.vProgram);
        if(state->settings.showMajorAxis)
            shaderData.majorIndex = vMajorIdx;
        glNamedBufferSubData(state->gl.editorBackProg.shaderDataBuffer, 0, sizeof shaderData, &shaderData);
        glDrawArraysInstanced(GL_LINES, 2, 2, vLines);
    }
    else if(state->settings.showMajorAxis)
    {
        shaderData.majorIndex = 0;
        shaderData.period = 0;
        shaderData.hOffset = -state->data.viewPosition.y;
        shaderData.vOffset = -state->data.viewPosition.x;

        glNamedBufferSubData(state->gl.editorBackProg.shaderDataBuffer, 0, sizeof shaderData, &shaderData);
        glUseProgram(state->gl.editorBackProg.hProgram);
        glDrawArraysInstanced(GL_LINES, 0, 2, 1);

        glUseProgram(state->gl.editorBackProg.vProgram);
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

static size_t CollectVertices(const EdState *state, size_t vertexOffset)
{
    size_t verts = 0;
    for(const MapVertex *vertex = state->map.headVertex; vertex; vertex = vertex->next)
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

        state->gl.editorVertexMap[verts + vertexOffset] = (EditorVertexType){ .position = vertex->pos, .color = state->settings.colors[colorIdx] };
        verts++;
    }
    return verts;
}

static size_t CollectLines(const EdState *state, size_t vertexOffset)
{
    size_t verts = 0;
    for(const MapLine *line = state->map.headLine; line; line = line->next)
    {
        int colorIdx = COL_LINE;
        if(line->frontSector && line->backSector)
        {
            colorIdx = COL_LINE_INNER;
        }
        if(state->data.selectionMode == MODE_LINE)
        {
            if(state->data.numSelectedElements > 0 && includes(state->data.selectedElements, state->data.numSelectedElements, line))
            {
                colorIdx = COL_LINE_SELECT;
            }
            else if(line == state->data.hoveredElement)
            {
                colorIdx = COL_LINE_HOVER;
            }
        }
        else if(state->data.selectionMode == MODE_SECTOR)
        {
            if(state->data.hoveredElement)
            {
                MapSector *sector = state->data.hoveredElement;
                if(includes((void**)sector->outerLines, sector->numOuterLines, line))
                {
                    colorIdx = COL_LINE_HOVER;
                    goto foundColIdx;
                }
            }
            for(size_t i = 0; i < state->data.numSelectedElements; ++i)
            {
                MapSector *sector = state->data.selectedElements[i];
                if(includes((void**)sector->outerLines, sector->numOuterLines, line))
                {
                    colorIdx = COL_LINE_SELECT;
                    break;
                }
            }
foundColIdx:
        }

        vec4s color = state->settings.colors[colorIdx];
        size_t relVertIdx = 0;
        state->gl.editorVertexMap[verts + vertexOffset + relVertIdx++] = (EditorVertexType){ .position = line->a->pos, .color = color };
        state->gl.editorVertexMap[verts + vertexOffset + relVertIdx++] = (EditorVertexType){ .position = line->b->pos, .color = color };

        vec2s dir = glms_vec2_sub(line->b->pos, line->a->pos);
        vec2s normalStart = glms_vec2_add(line->a->pos, glms_vec2_scale(dir, 0.5f));
        vec2s perpDir = glms_vec2_normalize((vec2s){ .x = -dir.y, .y = dir.x });

        float inverseZoom = 1.0f / (state->data.zoomLevel);
        float normalLength = 6;
        vec2s normalEnd = glms_vec2_add(normalStart, glms_vec2_scale(perpDir, normalLength * inverseZoom));

        state->gl.editorVertexMap[verts + vertexOffset + relVertIdx++] = (EditorVertexType){ .position = normalStart, .color = color };
        state->gl.editorVertexMap[verts + vertexOffset + relVertIdx++] = (EditorVertexType){ .position = normalEnd, .color = color };

        if(state->settings.showLineDir)
        {
            float arrowHeadThickness = 6;
            float arrowHeadHeight = 8;
            vec2s endPoint = glms_vec2_sub(line->b->pos, glms_vec2_scale(glms_vec2_normalize(dir), arrowHeadHeight));
            vec2s invPerpDir = { .x = -perpDir.x, .y = -perpDir.y };
            vec2s arrowHeadLeft = glms_vec2_add(endPoint, glms_vec2_scale(invPerpDir, arrowHeadThickness));
            vec2s arrowHeadRight = glms_vec2_add(endPoint, glms_vec2_scale(perpDir, arrowHeadThickness));

            state->gl.editorVertexMap[verts + vertexOffset + relVertIdx++] = (EditorVertexType){ .position = line->b->pos, .color = color };
            state->gl.editorVertexMap[verts + vertexOffset + relVertIdx++] = (EditorVertexType){ .position = arrowHeadLeft, .color = color };

            state->gl.editorVertexMap[verts + vertexOffset + relVertIdx++] = (EditorVertexType){ .position = line->b->pos, .color = color };
            state->gl.editorVertexMap[verts + vertexOffset + relVertIdx++] = (EditorVertexType){ .position = arrowHeadRight, .color = color };
        }
        verts += relVertIdx;
    }
    return verts;
}

typedef struct RenderData
{
    GLuint texture;
    size_t vertexOffset;
    size_t indexOffset;
    size_t indexCount;
} RenderData;

static size_t CollectSectors(const EdState *state, size_t vertexOffset, size_t indexOffset, RenderData *renderData, size_t renderDataSize, size_t *numTextures)
{
    size_t verts = 0, inds = 0, currentRenderData = 0;
    GLuint currentTexture = 0;
    RenderData *rd = &renderData[0];
    for(const MapSector *sector = state->map.headSector; sector; sector = sector->next)
    {
        int colorIdx = COL_SECTOR;
        /*
        if(state->data.numSelectedElements > 0 && includes(state->data.selectedElements, state->data.numSelectedElements, sector))
        {
            colorIdx = COL_SECTOR_SELECT;
        }
        else*/ if(sector == state->data.hoveredElement)
        {
            colorIdx = COL_SECTOR_HOVER;
        }
        const Texture *texture = tc_get(&state->textures, sector->data.floorTex);
        GLuint texId = texture ? texture->texture1 : state->defaultTextures.missingTexture;
        if(texId != currentTexture)
        {
            currentRenderData++;
            assert(currentRenderData < renderDataSize);
            currentTexture = texId;
            rd = &renderData[currentRenderData-1];
            rd->indexOffset = indexOffset + inds;
            rd->vertexOffset = vertexOffset + verts;
            rd->texture = texId;
        }

        const TriangleData data = sector->edData;
        for(size_t i = 0; i < data.numIndices; ++i)
        {
            state->gl.editorIndexMap[indexOffset + inds + i] = data.indices[i] + verts;
        }
        inds += data.numIndices;
        rd->indexCount += data.numIndices;

        size_t offsetIndex = verts + vertexOffset;
        for(size_t i = 0; i < data.numVertices; i++)
        {
            const vec2s position = data.vertices[i];
            const vec2s texcoord = data.vertices[i];
            
            state->gl.editorVertexMap[i + offsetIndex] = (EditorVertexType){ .position = position, .texCoord = texcoord, .color = state->settings.colors[colorIdx] };
        }
        verts += data.numVertices;
    }
    *numTextures = currentRenderData;
    return verts;
}

static size_t CollectDragData(const EdState *state, size_t vertexOffset)
{
    if(state->data.isDragging)
    {
        state->gl.editorVertexMap[vertexOffset + 0] = (EditorVertexType){ .position = state->data.editVertexDrag[0], .color = state->settings.colors[COL_ACTIVE_EDIT] };
        state->gl.editorVertexMap[vertexOffset + 1] = (EditorVertexType){ .position = state->data.editVertexDrag[1], .color = state->settings.colors[COL_ACTIVE_EDIT] };
        state->gl.editorVertexMap[vertexOffset + 2] = (EditorVertexType){ .position = state->data.editDragMouse, .color = state->settings.colors[COL_ACTIVE_EDIT] };
        state->gl.editorVertexMap[vertexOffset + 3] = (EditorVertexType){ .position = state->data.editVertexDrag[2], .color = state->settings.colors[COL_ACTIVE_EDIT] };
        return 4;
    }
    return 0;
}

static size_t CollectEditData(const EdState *state, size_t vertexOffset)
{
    if(state->data.editState == ESTATE_ADDVERTEX)
    {
        for(size_t i = 0; i < state->data.editVertexBufferSize; ++i)
        {
            state->gl.editorVertexMap[vertexOffset + i] = (EditorVertexType){ .position = state->data.editVertexBuffer[i], .color = state->settings.colors[COL_ACTIVE_EDIT] };
        }
        state->gl.editorVertexMap[vertexOffset + state->data.editVertexBufferSize] = (EditorVertexType){ .position = state->data.editVertexMouse, .color = state->settings.colors[COL_ACTIVE_EDIT] };

        return state->data.editVertexBufferSize + 1;
    }
    return 0;
}

void RenderEditorView(EdState *state)
{
    RenderBackground(state);

    mat4s viewMat = glms_translate_make((vec3s){{ -state->data.viewPosition.x, -state->data.viewPosition.y, 0 }});
    viewMat = glms_scale(viewMat, (vec3s){{ state->data.zoomLevel, state->data.zoomLevel, 1 }});
    mat4s viewProjMat = glms_mul(state->data.editorProjection, viewMat);

    if(state->gl.editorBufferFence[state->gl.currentBuffer] != NULL)
    {
        GLenum ret;
        while((ret = glClientWaitSync(state->gl.editorBufferFence[state->gl.currentBuffer], 0, 100)) == GL_TIMEOUT_EXPIRED);
        if(ret == GL_WAIT_FAILED) LogError("Fence wait failed\n");
    }

    glBindVertexArray(state->gl.editorVertexFormat);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, state->gl.editorShaderDataBuffer);

    EditorShaderData data =
    {
        .viewProj = viewProjMat,
        .tint = { .r = 1, .g = 1, .b = 1, .a = 1 }
    };
    glNamedBufferSubData(state->gl.editorShaderDataBuffer, 0, sizeof data, &data);

    size_t vertStart = state->gl.currentBuffer * state->gl.editorMaxBufferCount;
    size_t vertLength = CollectVertices(state, vertStart);
    size_t lineStart = vertStart + vertLength;
    size_t lineLength = CollectLines(state, lineStart);
    RenderData renderData[4096] = { 0 };
    size_t numTextures = 0;
    size_t sectorStart = lineStart + lineLength, sectorIndexStart = state->gl.currentBuffer * state->gl.editorMaxBufferCount;
    size_t sectorLength = CollectSectors(state, sectorStart, sectorIndexStart, renderData, 4096, &numTextures);
    size_t dragStart = sectorStart + sectorLength;
    size_t dragLength = CollectDragData(state, dragStart);
    size_t editStart = dragStart + dragLength;
    size_t editLength = CollectEditData(state, editStart);

    if(editStart + editLength == 0) // dont issue draw calls if we dont have anything to draw
        return;

    glUseProgram(state->gl.editorSector.program);
    glUniform1i(0, 0);
    for(size_t i = 0; i < numTextures; ++i)
    {
        glBindTextureUnit(0, renderData[i].texture);
        glDrawElementsBaseVertex(GL_TRIANGLES, renderData[i].indexCount, GL_UNSIGNED_INT, (void*)(renderData[i].indexOffset * sizeof(Index_t)), renderData[0].vertexOffset);
    }

    glLineWidth(2);
    glUseProgram(state->gl.editorLine.program);
    glDrawArrays(GL_LINES, lineStart, lineLength);

    glPointSize(state->settings.vertexPointSize);
    glUseProgram(state->gl.editorVertex.program);
    glDrawArrays(GL_POINTS, vertStart, vertLength);

    glUseProgram(state->gl.editorVertex.program);
    glDrawArrays(GL_POINTS, editStart, editLength);

    glUseProgram(state->gl.editorLine.program);
    glDrawArrays(GL_LINE_LOOP, dragStart, dragLength);
    glDrawArrays(GL_LINE_STRIP, editStart, editLength);

    glLineWidth(1);

    if(state->gl.editorBufferFence[state->gl.currentBuffer] != NULL) glDeleteSync(state->gl.editorBufferFence[state->gl.currentBuffer]);
    state->gl.editorBufferFence[state->gl.currentBuffer] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    state->gl.currentBuffer = (state->gl.currentBuffer + 1) % NUM_BUFFERS;
}
