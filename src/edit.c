#include "edit.h"
#include "map.h"

#include <triangulate.h>

static void IncreaseBufferSize(void **buffer, size_t *capacity, size_t elementSize);

static int32_t sign(int32_t x) {
    return (x > 0) - (x < 0);
}

void ScreenToEditorSpace(const struct EdState *state, int32_t *x, int32_t *y)
{
    const float z = state->data.zoomLevel;
    *x = (int32_t)((*x + state->data.viewPosition.x) / z);
    *y = (int32_t)((*y + state->data.viewPosition.y) / z);
}

void ScreenToEditorSpacef(const struct EdState *state, float *x, float *y)
{
    const float z = state->data.zoomLevel;
    *x = (*x + state->data.viewPosition.x) / z;
    *y = (*y + state->data.viewPosition.y) / z;
}

void EditorToScreenSpace(const struct EdState *state, int32_t *x, int32_t *y)
{
    const float z = state->data.zoomLevel;
    *x = (int32_t)((*x - state->data.viewPosition.x) * z);
    *y = (int32_t)((*y - state->data.viewPosition.y) * z);
}

void ScreenToEditorSpaceGrid(const struct EdState *state, int32_t *x, int32_t *y)
{
    ScreenToEditorSpace(state, x, y);
    const int offset = state->data.gridSize / 2;
    int32_t xt = *x, yt = *y;
    xt += xt < 0 ? -offset : offset;
    *x = xt / state->data.gridSize * state->data.gridSize;
    yt += yt < 0 ? -offset : offset;
    *y = yt / state->data.gridSize * state->data.gridSize;
}

void EditCopy(struct EdState *state)
{
    LogInfo("Copy!!\n");
}

void EditPaste(struct EdState *state)
{
    LogInfo("Paste!!\n");
}

void EditCut(struct EdState *state)
{
    LogInfo("Cut!!\n");
}

ssize_t EditAddVertex(struct EdState *state, struct Vertex pos)
{
    struct Map *map = &state->map;
    for(size_t i = 0; i < map->numVertices; ++i)
    {
        const struct Vertex v = map->vertices[i];
        if(v.x == pos.x && v.y == pos.y) return i;
    }

    size_t idx = map->numVertices++;
    map->vertices[idx] = pos;
    if(map->numVertices == map->numAllocVertices)
        IncreaseBufferSize((void**)&map->vertices, &map->numAllocVertices, sizeof *map->vertices);

    state->gl.editorVertex.bufferMap[idx] = (struct VertexType){ .position = { pos.x, pos.y }, .color = { 1, 1, 1, 1 } };

    map->dirty = true;
    return idx;
}

void EditRemoveVertex(struct EdState *state, size_t index)
{
    struct Map *map = &state->map;
    if(index >= map->numVertices) return;

    // shift everything above index down
    for(size_t i = index + 1; i < map->numVertices; ++i)
    {
        map->vertices[i-1] = map->vertices[i];
    }
    map->numVertices--;

    // remove lines affected by vertex

    map->dirty = true;
}

bool EditGetVertex(struct EdState *state, struct Vertex pos, size_t *ind)
{
    struct Map *map = &state->map;
    for(size_t i = 0; i < map->numVertices; ++i)
    {
        const struct Vertex v = map->vertices[i];
        if(v.x == pos.x && v.y == pos.y)
        {
            if(ind) *ind = i;
            return true;
        }
    }
    return false;
}

ssize_t EditAddLine(struct EdState *state, size_t v0, size_t v1)
{
    struct Map *map = &state->map;
    if(v0 >= map->numVertices || v1 >= map->numVertices || v0 == v1) return -1;

    for(size_t i = 0; i < map->numLines; ++i)
    {
        const struct Line l = map->lines[i];
        bool ab = l.a == v0 && l.b == v1;
        bool ba = l.a == v1 && l.b == v0;
        if(ab || ba) return i;
    }

    const struct Vertex vert0 = map->vertices[v0];
    const struct Vertex vert1 = map->vertices[v1];
    int32_t normal = sign((vert0.x*vert1.y) - (vert0.y*vert1.x));

    size_t idx = map->numLines++;
    map->lines[idx] = (struct Line){ .a = v0, .b = v1, .type = ST_NORMAL, .normal = normal };
    if(map->numLines == map->numAllocLines)
        IncreaseBufferSize((void**)&map->lines, &map->numAllocLines, sizeof *map->lines);

    vec2 line;
    glm_vec2_sub((vec2){ vert1.x, vert1.y }, (vec2){ vert0.x, vert0.y }, line);
    vec2 n = { -line[1], line[0] };
    glm_vec2_normalize(n);

    vec2 middle;
    glm_vec2_scale(line, 0.5f, middle);
    glm_vec2_add((vec2){ vert0.x, vert0.y }, middle, middle);

    vec2 middleNormal;
    glm_vec2_scale(n, 10.0f, middleNormal);
    glm_vec2_add(middle, middleNormal, middleNormal);

    state->gl.editorLine.bufferMap[idx * 4    ] = (struct VertexType){ .position = { vert0.x, vert0.y }, .color = { 1, 1, 1, 1 } };
    state->gl.editorLine.bufferMap[idx * 4 + 1] = (struct VertexType){ .position = { vert1.x, vert1.y }, .color = { 1, 1, 1, 1 } };
    state->gl.editorLine.bufferMap[idx * 4 + 2] = (struct VertexType){ .position = { middle[0], middle[1] }, .color = { 1, 1, 1, 1 } };
    state->gl.editorLine.bufferMap[idx * 4 + 3] = (struct VertexType){ .position = { middleNormal[0], middleNormal[1] }, .color = { 1, 1, 1, 1 } };

    map->dirty = true;
    return idx;
}

void EditRemoveLine(struct EdState *state, size_t index)
{
    struct Map *map = &state->map;
    map->dirty = true;
}

bool EditGetLine(struct EdState *state, struct Vertex pos, size_t *ind)
{
    return false;
}

ssize_t EditAddSector(struct EdState *state, size_t *lineIndices, size_t numLines)
{
    struct Map *map = &state->map;

    for(size_t i = 0; i < map->numSectors; ++i)
    {
        struct Sector s = map->sectors[i];
        if(s.numLines != numLines) continue;
        for(size_t j = 0; j < s.numLines; ++j)
        {
            //
        }
    }

    size_t idx = map->numSectors++;
    struct Sector *s = &map->sectors[idx];
    s->lines = calloc(numLines, sizeof *s->lines);
    s->numLines = numLines;
    for(size_t i = 0; i < numLines; ++i)
        s->lines[i] = lineIndices[i];

    if(map->numSectors == map->numAllocSectors)
    {
        IncreaseBufferSize((void**)&map->sectors, &map->numAllocSectors, sizeof *map->sectors);
        state->sectorToPolygon = realloc(state->sectorToPolygon, map->numAllocSectors * sizeof *state->sectorToPolygon);
    }

    size_t baseVertexIndex = state->gl.editorSector.highestVertIndex;
    size_t baseIndexIndex = state->gl.editorSector.highestIndIndex;

    size_t index = baseVertexIndex;
    struct Line lastLine;
    size_t lastIndex;
    struct Polygon *outerPolygon = calloc(1, sizeof *outerPolygon + numLines * sizeof *outerPolygon->vertices);
    outerPolygon->length = numLines;
    for(size_t i = 0; i < numLines; ++i)
    {
        struct Line line = map->lines[lineIndices[i]];
        size_t vertIdx;
        if(i == 0)
            vertIdx = line.a;
        else 
        {
            size_t lastEnd = lastLine.a == lastIndex ? lastLine.b : lastLine.a;
            vertIdx = line.a == lastEnd ? line.a : line.b;
        }
        lastLine = line;
        lastIndex = vertIdx;

        int32_t x = map->vertices[vertIdx].x;
        int32_t y = map->vertices[vertIdx].y;
        state->gl.editorSector.bufferMap[index++] = (struct SectorVertexType){ .position = { x, y }, .color = { 1, 1, 1, 1 }, .texCoord = { 0, 0 } };
        outerPolygon->vertices[i][0] = x;
        outerPolygon->vertices[i][1] = y;
    }
    unsigned int *indices = NULL;
    unsigned long numIndices = triangulate(outerPolygon, NULL, 0, &indices);

    index = baseIndexIndex;
    for(size_t i = 0; i < numIndices; ++i)
        state->gl.editorSector.indexMap[index++] = indices[i] + baseVertexIndex;

    free(outerPolygon);
    free(indices);

    state->sectorToPolygon[idx] = (__typeof__(*state->sectorToPolygon)){ .indexStart = baseIndexIndex, .indexLength = numIndices, .vertexStart = baseVertexIndex, .vertexLength = numLines };

    state->gl.editorSector.highestVertIndex += numLines;
    state->gl.editorSector.highestIndIndex += numIndices;

    map->dirty = true;
    return idx;
}

void EditRemoveSector(struct EdState *state, size_t index)
{
    struct Map *map = &state->map;
    map->dirty = true;
}

bool EditGetSector(struct EdState *state, struct Vertex pos, size_t *ind)
{
    return false;
}

ssize_t EditApplyLines(struct EdState *state, struct Vertex *points, size_t num)
{

}

ssize_t EditApplySector(struct EdState *state, struct Vertex *points, size_t num)
{
    struct Polygon *sourcePoly = malloc(sizeof *sourcePoly + num * sizeof *sourcePoly->vertices);
    sourcePoly->length = num;
    for(size_t i = 0; i < num; ++i)
    {
        sourcePoly->vertices[i][0] = points[i].x;
        sourcePoly->vertices[i][1] = points[i].y;
    }

    struct Polygon **sourceSimples;
    size_t numSimples = makeSimple(sourcePoly, &sourceSimples);

    LogInfo("Make simple: {d}", numSimples);

    if(state->map.numSectors == 0)
    {

    }
    else
    {

    }

    free(sourcePoly);
    freePolygons(sourceSimples, numSimples);
    free(sourceSimples);

    return -1;
}

static void IncreaseBufferSize(void **buffer, size_t *capacity, size_t elementSize)
{
    *capacity = (*capacity) * 2;
    *buffer = realloc(*buffer, (*capacity) * elementSize);
}
