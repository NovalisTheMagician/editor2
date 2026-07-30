#include "editor.h"

#include <string.h>

#include <cglm/struct.h>
#include <cglm/struct/cam.h>
#include <cglm/util.h>

#include "arena.h"

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

static Arena renderArena = { 0 };

typedef struct RenderData
{
    GLuint texture;
    Index_t *items;
    size_t count, capacity;
} RenderData;

typedef struct RenderList
{
    RenderData *items;
    size_t count, capacity;
} RenderList;

static RenderData* GetRenderDataSlot(RenderList *renderList, GLuint texture)
{
    for(size_t i = 0; i < renderList->count; ++i)
    {
        RenderData *data = &renderList->items[i];
        if(data->texture == texture)
            return data;
    }

    RenderData data = { .texture = texture };
    arena_da_append(&renderArena, renderList, data);
    return &renderList->items[renderList->count - 1];
}

static GLuint GetTextureId(const EdState *state, const Texture *texture)
{
    return texture ? texture->texture1 : state->defaultTextures.missingTexture;
}

static size_t CollectFlats(const EdState *state, size_t vertexOffset, RenderList *renderList)
{
    size_t verts = 0;
    for(const MapSector *sector = state->map.headSector; sector; sector = sector->next)
    {
        real_t light = sector->data.lightLevel / 255.0f;
        const TriangleData data = sector->edData;

        real_t z = sector->data.floorHeight;
        GLuint texId = GetTextureId(state, tc_get(&state->textures, sector->data.floorTex));
        RenderData *rd = GetRenderDataSlot(renderList, texId);

        for(size_t i = 0; i < data.numIndices; ++i)
            arena_da_append(&renderArena, rd, data.indices[data.numIndices - i - 1] + verts + vertexOffset);

        size_t offsetIndex = verts + vertexOffset;
        for(size_t i = 0; i < data.numVertices; ++i)
        {
            const Vec2 ePos = data.vertices[i];
            const Vec3 position = { .x = ePos.x, .y = z, .z = ePos.y };
            const Vec2 texcoord = vec2_scale(data.vertices[i], 1.0f / state->map.textureScale);
            const Color color = { light, light, light, 1 };

            state->gl.realtimeVertexMap[i + offsetIndex] = (RealtimeVertexType){ .position = position, .texCoord = texcoord, .color = color };
        }
        verts += data.numVertices;

        z = sector->data.ceilHeight;
        texId = GetTextureId(state, tc_get(&state->textures, sector->data.ceilTex));

        rd = GetRenderDataSlot(renderList, texId);
        
        for(size_t i = 0; i < data.numIndices; ++i)
            arena_da_append(&renderArena, rd, data.indices[i] + verts + vertexOffset);

        offsetIndex = verts + vertexOffset;
        for(size_t i = 0; i < data.numVertices; ++i)
        {
            const Vec2 ePos = data.vertices[i];
            const Vec3 position = { .x = ePos.x, .y = z, .z = ePos.y };
            const Vec2 texcoord = vec2_scale(data.vertices[i], 1.0 / state->map.textureScale);
            const Color color = { light, light, light, 1 };

            state->gl.realtimeVertexMap[i + offsetIndex] = (RealtimeVertexType){ .position = position, .texCoord = texcoord, .color = color };
        }
        verts += data.numVertices;
    }
    return verts;
}

static void MakeWall(Vec2 a, Vec2 b, real_t zb, real_t zt, RealtimeVertexType *vertices, RenderData *renderData, size_t vertOffset, real_t texScale, real_t yOffset, real_t light)
{
    real_t len = vec2_distance(a, b);
    real_t height = zt - zb;
    const Vec3 tl = { .x = a.x, .y = zt, .z = a.y };
    const Vec3 tr = { .x = b.x, .y = zt, .z = b.y };
    const Vec3 bl = { .x = a.x, .y = zb, .z = a.y };
    const Vec3 br = { .x = b.x, .y = zb, .z = b.y };
    const Vec2 uvtl = vec2_scale((Vec2){ 0, yOffset }, texScale), uvtr = vec2_scale((Vec2){ len, yOffset }, texScale);
    const Vec2 uvbl = vec2_scale((Vec2){ 0, height + yOffset }, texScale), uvbr = vec2_scale((Vec2){ len, height + yOffset }, texScale);
    const Color color = { light, light, light, 1.0f };
    
    vertices[0] = (RealtimeVertexType){ .position = tl, .texCoord = uvtl, .color = color };
    vertices[1] = (RealtimeVertexType){ .position = tr, .texCoord = uvtr, .color = color };
    vertices[2] = (RealtimeVertexType){ .position = bl, .texCoord = uvbl, .color = color };
    vertices[3] = (RealtimeVertexType){ .position = br, .texCoord = uvbr, .color = color };

    Index_t localIndices[6] = { 2, 1, 0, 3, 1, 2 };
    for(size_t i = 0; i < 6; ++i)
        arena_da_append(&renderArena, renderData, localIndices[i] + vertOffset);
}

static size_t CollectWalls(const EdState *state, size_t vertexOffset, RenderList *renderList)
{
    size_t verts = 0;
    real_t scaleValue = 1.0 / state->map.textureScale;
    for(const MapLine *line = state->map.headLine; line; line = line->next)
    {
        LineData data = line->data;
        if(line->frontSector && line->backSector) // double sided line -> uses upper, lower and mid textures
        {
            bool frontBottomExposed = line->frontSector->data.floorHeight < line->backSector->data.floorHeight;
            bool frontTopExposed = line->frontSector->data.ceilHeight > line->backSector->data.ceilHeight;

            real_t frontLight = line->frontSector->data.lightLevel / 255.0f;
            real_t backLight = line->backSector->data.lightLevel / 255.0f;

            real_t wzb = max(line->frontSector->data.floorHeight, line->backSector->data.floorHeight);
            real_t wzt = min(line->frontSector->data.ceilHeight, line->backSector->data.ceilHeight);
            real_t windowHeight = wzt - wzb;

            real_t frontTopHeight = line->frontSector->data.ceilHeight - line->backSector->data.ceilHeight;

            real_t backTopHeight = line->backSector->data.ceilHeight - line->frontSector->data.ceilHeight;

            bool noBottomExpose = false;
            if(line->frontSector->data.floorHeight == line->backSector->data.floorHeight) // no lower texture needed here
                noBottomExpose = true;
            bool noTopExpose = false;
            if(line->frontSector->data.ceilHeight == line->backSector->data.ceilHeight) // no lower texture needed here
                noTopExpose = true;

            Vec2 a = vec2_from_fvec2(line->a->pos), b = vec2_from_fvec2(line->b->pos);
            if(!noBottomExpose)
            {
                if(frontBottomExposed)
                {
                    real_t zb = line->frontSector->data.floorHeight;
                    real_t zt = line->backSector->data.floorHeight;
                    GLuint texId = GetTextureId(state, tc_get(&state->textures, data.front.lowerTex));
                    RenderData *rd = GetRenderDataSlot(renderList, texId);
                    MakeWall(a, b, zb, zt, state->gl.realtimeVertexMap + vertexOffset + verts, rd, vertexOffset + verts, scaleValue, windowHeight + frontTopHeight, frontLight);
                    verts += 4;
                }
                else
                {
                    real_t zb = line->backSector->data.floorHeight;
                    real_t zt = line->frontSector->data.floorHeight;
                    GLuint texId = GetTextureId(state, tc_get(&state->textures, data.back.lowerTex));
                    RenderData *rd = GetRenderDataSlot(renderList, texId);
                    MakeWall(b, a, zb, zt, state->gl.realtimeVertexMap + vertexOffset + verts, rd, vertexOffset + verts, scaleValue, windowHeight + backTopHeight, backLight);
                    verts += 4;
                }
            }

            if(!noTopExpose)
            {
                if(frontTopExposed)
                {
                    real_t zt = line->frontSector->data.ceilHeight;
                    real_t zb = line->backSector->data.ceilHeight;
                    GLuint texId = GetTextureId(state, tc_get(&state->textures, data.front.upperTex));
                    RenderData *rd = GetRenderDataSlot(renderList, texId);
                    MakeWall(a, b, zb, zt, state->gl.realtimeVertexMap + vertexOffset + verts, rd, vertexOffset + verts, scaleValue, 0, frontLight);
                    verts += 4;
                }
                else
                {
                    real_t zt = line->backSector->data.ceilHeight;
                    real_t zb = line->frontSector->data.ceilHeight;
                    GLuint texId = GetTextureId(state, tc_get(&state->textures, data.back.upperTex));
                    RenderData *rd = GetRenderDataSlot(renderList, texId);
                    MakeWall(b, a, zb, zt, state->gl.realtimeVertexMap + vertexOffset + verts, rd, vertexOffset + verts, scaleValue, 0, backLight);
                    verts += 4;
                }
            }

            if(data.front.middleTex)
            {
                GLuint texId = GetTextureId(state, tc_get(&state->textures, data.front.middleTex));
                RenderData *rd = GetRenderDataSlot(renderList, texId);
                MakeWall(a, b, wzb, wzt, state->gl.realtimeVertexMap + vertexOffset + verts, rd, vertexOffset + verts, scaleValue, frontTopHeight, frontLight);
                verts += 4;
            }
            if(data.back.middleTex)
            {
                GLuint texId = GetTextureId(state, tc_get(&state->textures, data.back.middleTex));
                RenderData *rd = GetRenderDataSlot(renderList, texId);
                MakeWall(b, a, wzb, wzt, state->gl.realtimeVertexMap + vertexOffset + verts, rd, vertexOffset + verts, scaleValue, backTopHeight, backLight);
                verts += 4;
            }
        }
        else if(!line->frontSector && !line->backSector) // line not part of a sector -> need to check if within a sector bounds
        {
            //assert(false && "Unhandled wall branch");
        }
        else // single sided line -> only uses mid textures
        {
            bool front = line->frontSector != NULL;
            Vec2 a = vec2_from_fvec2(line->a->pos), b = vec2_from_fvec2(line->b->pos);

            if(front)
            {
                real_t light = line->frontSector->data.lightLevel / 255.0f;
                real_t zb = line->frontSector->data.floorHeight;
                real_t zt = line->frontSector->data.ceilHeight;
                GLuint texId = GetTextureId(state, tc_get(&state->textures, data.front.middleTex));
                RenderData *rd = GetRenderDataSlot(renderList, texId);
                MakeWall(a, b, zb, zt, state->gl.realtimeVertexMap + vertexOffset + verts, rd, vertexOffset + verts, scaleValue, 0, light);
            }
            else
            {
                real_t light = line->backSector->data.lightLevel / 255.0f;
                real_t zb = line->backSector->data.floorHeight;
                real_t zt = line->backSector->data.ceilHeight;
                GLuint texId = GetTextureId(state, tc_get(&state->textures, data.back.middleTex));
                RenderData *rd = GetRenderDataSlot(renderList, texId);
                MakeWall(b, a, zb, zt, state->gl.realtimeVertexMap + vertexOffset + verts, rd, vertexOffset + verts, scaleValue, 0, light);
            }
            verts += 4;
        }
    }
    return verts;
}

void RenderRealtimeView(EdState *state)
{
    Vec3 center = vec3_add(state->realtime.cameraPosition, state->realtime.cameraDirection);
    Vec3 up = vec3_cross(state->realtime.cameraRight, state->realtime.cameraDirection);
    float fov = glm_rad(state->settings.realtimeFov);
    float aspect = (float)state->gl.realtimeFramebufferWidth / state->gl.realtimeFramebufferHeight;
    mat4s projection = glms_perspective(fov, aspect, 0.01f, 1000.0f);
    mat4s view = glms_lookat(*(vec3s*)&state->realtime.cameraPosition, *(vec3s*)&center, *(vec3s*)&up);
    mat4s viewProj = glms_mul(projection, view);

    if(state->gl.editorBufferFence[state->gl.currentBuffer] != NULL)
    {
        GLenum ret;
        while((ret = glClientWaitSync(state->gl.editorBufferFence[state->gl.currentBuffer], 0, 100)) == GL_TIMEOUT_EXPIRED);
        if(ret == GL_WAIT_FAILED) LogError("Fence wait failed\n");
    }

    glBindVertexArray(state->gl.realtimeVertexFormat);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, state->gl.editorShaderDataBuffer);

    EditorShaderData data =
    {
        .viewProj = viewProj,
        .tint = { .r = 1, .g = 1, .b = 1, .a = 1 },
        .zoom = state->data.zoomLevel
    };
    glNamedBufferSubData(state->gl.editorShaderDataBuffer, 0, sizeof data, &data);

    RenderList renderList = { 0 };

    size_t flatsStart = state->gl.currentBuffer * state->gl.editorMaxBufferCount;
    size_t flatsLength = CollectFlats(state, flatsStart, &renderList);

    size_t wallsStart = flatsStart + flatsLength;
    size_t wallsLength = CollectWalls(state, wallsStart, &renderList);

    glUseProgram(state->gl.realtimeProgram.program);
    glUniform1i(0, 0);
    size_t offset = state->gl.currentBuffer * state->gl.editorMaxBufferCount;
    for(size_t i = 0; i < renderList.count; ++i)
    {
        RenderData *rd = &renderList.items[i];
        glBindTextureUnit(0, rd->texture);
        memcpy(state->gl.realtimeIndexMap + offset, rd->items, rd->count * sizeof *rd->items);
        glDrawElements(GL_TRIANGLES, rd->count, GL_UNSIGNED_INT, (void*)(offset * sizeof(Index_t)));
        offset += rd->count;
    }

    arena_reset(&renderArena);

    if(state->gl.editorBufferFence[state->gl.currentBuffer] != NULL) glDeleteSync(state->gl.editorBufferFence[state->gl.currentBuffer]);
    state->gl.editorBufferFence[state->gl.currentBuffer] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    state->gl.currentBuffer = (state->gl.currentBuffer + 1) % NUM_BUFFERS;
}
