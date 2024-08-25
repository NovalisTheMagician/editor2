#include "edit.h"
#include "map.h"

#include "geometry.h"

#include <assert.h>
#include <tgmath.h>

#include <triangulate.h>

#include "map/remove.h"
#include "map/util.h"
#include "map/insert.h"
#include "map/create.h"

void ScreenToEditorSpace(const struct EdState state[static 1], int32_t x[static 1], int32_t y[static 1])
{
    const float z = state->data.zoomLevel;
    *x = (int32_t)((*x + state->data.viewPosition.x) / z);
    *y = (int32_t)((*y + state->data.viewPosition.y) / z);
}

void ScreenToEditorSpacef(const struct EdState state[static 1], float x[static 1], float y[static 1])
{
    const float z = state->data.zoomLevel;
    *x = (*x + state->data.viewPosition.x) / z;
    *y = (*y + state->data.viewPosition.y) / z;
}

void EditorToScreenSpace(const struct EdState state[static 1], int32_t x[static 1], int32_t y[static 1])
{
    const float z = state->data.zoomLevel;
    *x = (int32_t)((*x - state->data.viewPosition.x) * z);
    *y = (int32_t)((*y - state->data.viewPosition.y) * z);
}

void ScreenToEditorSpaceGrid(const struct EdState state[static 1], int32_t x[static 1], int32_t y[static 1])
{
    ScreenToEditorSpace(state, x, y);
    const int offset = state->data.gridSize / 2;
    int32_t xt = *x, yt = *y;
    xt += xt < 0 ? -offset : offset;
    *x = xt / state->data.gridSize * state->data.gridSize;
    yt += yt < 0 ? -offset : offset;
    *y = yt / state->data.gridSize * state->data.gridSize;
}

void EditCopy(struct EdState state[static 1])
{
    LogDebug("Copy!!\n");
}

void EditPaste(struct EdState state[static 1])
{
    LogDebug("Paste!!\n");
}

void EditCut(struct EdState state[static 1])
{
    LogDebug("Cut!!\n");
}

struct MapVertex* EditAddVertex(struct EdState state[static 1], vec2s pos)
{
    struct Map *map = &state->map;
    struct CreateResult result = CreateVertex(map, pos);
    if(!result.created) return result.mapElement;
    struct MapVertex *vertex = result.mapElement;

    state->gl.editorVertex.bufferMap[vertex->idx] = (struct VertexType){ .position = pos, .color = { 1, 1, 1, 1 } };

    return vertex;
}

void EditRemoveVertices(struct EdState state[static 1], size_t num, struct MapVertex *vertices[static num])
{
    struct Map *map = &state->map;
    
    struct MapLine *potentialLines[4096] = { 0 };
    size_t numPotentialLines = 0;

    for(size_t i = 0; i < num; ++i)
    {
        struct MapVertex *vertex = vertices[i];
        for(size_t i = 0; i < vertex->numAttachedLines; ++i)
        {
            bool lineIsInSet = false;
            struct MapLine *attLine = vertex->attachedLines[i];
            for(size_t j = 0; j < numPotentialLines; ++j)
            {
                lineIsInSet |= potentialLines[j] == attLine;
                if(lineIsInSet) break;
            }

            if(!lineIsInSet) potentialLines[numPotentialLines++] = attLine;
        }
        RemoveVertex(map, vertex);
    }

    for(size_t i = 0; i < numPotentialLines; ++i)
    {
        struct MapLine *line = potentialLines[i];
        if(!line->a || !line->b)
            RemoveLine(map, line);
    }

    map->dirty = true;
}

struct MapVertex* EditGetVertex(struct EdState state[static 1], vec2s pos)
{
    struct Map *map = &state->map;
    for(struct MapVertex *vertex = map->headVertex; vertex; vertex = vertex->next)
    {
        if(vertex->pos.x == pos.x && vertex->pos.y == pos.y)
        {
            return vertex;
        }
    }
    return NULL;
}

struct MapVertex* EditGetClosestVertex(struct EdState state[static 1], vec2s pos, float maxDist)
{
    struct Map *map = &state->map;
    struct MapVertex *closestVertex = NULL;
    float closestDist = FLT_MAX;
    for(struct MapVertex *vertex = map->headVertex; vertex; vertex = vertex->next)
    {
        float dist2 = glms_vec2_distance2(vertex->pos, pos);
        if(dist2 <= maxDist*maxDist)
        {
            float dist = sqrt(dist2);
            if(dist < closestDist)
            {
                closestDist = dist;
                closestVertex = vertex;
            }
        }
    }
    return closestVertex;
}

struct MapLine* EditAddLine(struct EdState state[static 1], struct MapVertex v0[static 1], struct MapVertex v1[static 1], struct LineData data)
{
    struct Map *map = &state->map;

    struct CreateResult result = CreateLine(map, v0, v1, data);
    if(!result.created) return result.mapElement;
    struct MapLine *line = result.mapElement;

    vec2s vert0 = line->a->pos;
    vec2s vert1 = line->b->pos;

    vec2s l = glms_vec2_sub(vert1, vert0);
    vec2s n = {{ -l.y, l.x }};
    n = glms_vec2_normalize(n);

    vec2s middle = glms_vec2_scale(l, 0.5f);
    middle = glms_vec2_add(vert0, middle);

    vec2s middleNormal = glms_vec2_scale(n, 10.0f);
    middleNormal = glms_vec2_add(middle, middleNormal);

    state->gl.editorLine.bufferMap[line->idx * 4    ] = (struct VertexType){ .position = vert0, .color = { 1, 1, 1, 1 } };
    state->gl.editorLine.bufferMap[line->idx * 4 + 1] = (struct VertexType){ .position = vert1, .color = { 1, 1, 1, 1 } };
    state->gl.editorLine.bufferMap[line->idx * 4 + 2] = (struct VertexType){ .position = middle, .color = { 1, 1, 1, 1 } };
    state->gl.editorLine.bufferMap[line->idx * 4 + 3] = (struct VertexType){ .position = middleNormal, .color = { 1, 1, 1, 1 } };

    return line;
}

void EditRemoveLines(struct EdState state[static 1], size_t num, struct MapLine *lines[static num])
{
    struct Map *map = &state->map;

    struct MapVertex *potentialVertices[4096] = { 0 };
    size_t numPotentialVertices = 0;

    for(size_t i = 0; i < num; ++i)
    {
        struct MapLine *line = lines[i];
        bool isAInSet = false;
        bool isBInSet = false;
        for(size_t i = 0; i < numPotentialVertices; ++i)
        {
            if(!isAInSet) isAInSet = potentialVertices[i] == line->a;
            if(!isBInSet) isBInSet = potentialVertices[i] == line->b;
            if(isAInSet && isBInSet) break;
        }
        if(!isAInSet) potentialVertices[numPotentialVertices++] = line->a;
        if(!isBInSet) potentialVertices[numPotentialVertices++] = line->b;

        RemoveLine(map, line);
    }

    for(size_t i = 0; i < numPotentialVertices; ++i)
    {
        struct MapVertex *vertex = potentialVertices[i];
        if(vertex->numAttachedLines == 0) RemoveVertex(map, vertex);
    }

    map->dirty = true;
}

struct MapLine* EditGetClosestLine(struct EdState state[static 1], vec2s pos, float maxDist)
{
    struct Map *map = &state->map;
    struct MapLine *closestLine = NULL;
    float closestDist = FLT_MAX;
    for(struct MapLine *line = map->headLine; line; line = line->next)
    {
        float dist = MinDistToLine(line->a->pos, line->b->pos, pos);
        if(dist <= maxDist && dist < closestDist)
        {
            closestDist = dist;
            closestLine = line;
        }
    }
    return closestLine;
}

struct MapSector* EditAddSector(struct EdState *state, size_t numLines, struct MapLine *lines[static numLines], bool lineFronts[static numLines], struct SectorData data)
{
    struct Map *map = &state->map;

    struct CreateResult result = CreateSector(map, numLines, lines, lineFronts, data);
    if(!result.created) return result.mapElement;
    struct MapSector *sector = result.mapElement;

    struct Polygon *polygon = PolygonFromMapLines(numLines, lines, lineFronts);

    sector->vertices = calloc(polygon->length, sizeof *sector->vertices);
    memcpy(sector->vertices, polygon->vertices, polygon->length * sizeof *polygon->vertices);

    size_t baseVertexIndex = state->gl.editorSector.highestVertIndex;
    size_t baseIndexIndex = state->gl.editorSector.highestIndIndex;

    size_t index = baseVertexIndex;
    for(size_t i = 0; i < polygon->length; ++i)
    {
        float x = polygon->vertices[i][0];
        float y = polygon->vertices[i][1];
        state->gl.editorSector.bufferMap[index++] = (struct SectorVertexType){ .position = {{ x, y }}, .color = { 1, 1, 1, 1 }, .texCoord = {{ 0, 0 }} };
    }
    unsigned int *indices = NULL;
    size_t numIndices = triangulate(polygon, NULL, 0, &indices);

    index = baseIndexIndex;
    for(size_t i = 0; i < numIndices; ++i)
        state->gl.editorSector.indexMap[index++] = indices[i] + baseVertexIndex;

    free(indices);

    sector->edData = (struct TriangleData){ .indexStart = baseIndexIndex, .indexLength = numIndices, .vertexStart = baseVertexIndex, .vertexLength = polygon->length };

    state->gl.editorSector.highestVertIndex += polygon->length;
    state->gl.editorSector.highestIndIndex += numIndices;

    sector->bb = BoundingBoxFromVertices(polygon->length, sector->vertices);
    free(polygon);

    return sector;
}

void EditRemoveSectors(struct EdState state[static 1], size_t num, struct MapSector *sectors[static num])
{
    struct Map *map = &state->map;

    for(size_t i = 0; i < num; ++i)
    {
        struct MapSector *sector = sectors[i];
        RemoveSector(map, sector);
    }

    map->dirty = true;
}

struct MapSector* EditGetSector(struct EdState state[static 1], vec2s pos)
{
    struct Map *map = &state->map;

    for(struct MapSector *sector = map->headSector; sector; sector = sector->next)
    {
        struct 
        {
            struct MapSector *s[1000];
            int top;
        } stack = { .s = { sector }, .top = 0 };

        while(stack.top >= 0)
        {
            struct MapSector *sectorToCheck = stack.s[stack.top--];
            if(sectorToCheck->numContains > 0)
            {
                stack.s[++stack.top] = sectorToCheck;
                for(size_t i = 0; i < sectorToCheck->numContains; ++i)
                {
                    stack.s[++stack.top] = sectorToCheck->contains[i];
                }
                continue;
            }

            if(PointInSector(sectorToCheck, pos))
                return sectorToCheck;
        }
    }

    return NULL;
}

struct MapLine* EditApplyLines(struct EdState state[static 1], size_t num, vec2s points[static num])
{
    InsertLinesIntoMap(state, num, points, false);
    return NULL;
}

struct MapSector* EditApplySector(struct EdState state[static 1], size_t num, vec2s points[static num])
{
    InsertLinesIntoMap(state, num, points, true);
    return NULL;
}
