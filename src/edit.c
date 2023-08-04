#include "edit.h"
#include "map.h"

static void IncreaseBufferSize(void **buffer, size_t *capacity, size_t elementSize);

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
    printf("Copy!!\n");
}

void EditPaste(struct EdState *state)
{
    printf("Paste!!\n");
}

void EditCut(struct EdState *state)
{
    printf("Cut!!\n");
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
            *ind = i;
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

    size_t idx = map->numLines++;
    map->lines[idx] = (struct Line){ .a = v0, .b = v1, .type = ST_NORMAL };
    if(map->numLines == map->numAllocLines)
        IncreaseBufferSize((void**)&map->lines, &map->numAllocLines, sizeof *map->lines);

    const struct Vertex vert0 = map->vertices[v0];
    const struct Vertex vert1 = map->vertices[v1];
    state->gl.editorLine.bufferMap[idx * 2    ] = (struct VertexType){ .position = { vert0.x, vert0.y }, .color = { 1, 1, 1, 1 } };
    state->gl.editorLine.bufferMap[idx * 2 + 1] = (struct VertexType){ .position = { vert1.x, vert1.y }, .color = { 1, 1, 1, 1 } };

    map->dirty = true;

    return idx;
}

void EditRemoveLine(struct EdState *state, size_t index)
{
    struct Map *map = &state->map;
    map->dirty = true;
}

static void IncreaseBufferSize(void **buffer, size_t *capacity, size_t elementSize)
{
    *capacity = (*capacity) * 2;
    *buffer = realloc(*buffer, (*capacity) * elementSize);
}
