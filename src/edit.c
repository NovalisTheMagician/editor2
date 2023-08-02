#include "edit.h"
#include "map.h"

static void IncreaseBufferSize(void **buffer, size_t *capacity, size_t elementSize);

void ScreenToEditorSpace(const struct EdState *state, int32_t *x, int32_t *y)
{
    const float z = state->data.zoomLevel;
    *x = (int32_t)((*x + state->data.viewPosition.x) / z);
    *y = (int32_t)((*y + state->data.viewPosition.y) / z);
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

void EditAddVertex(struct EdState *state, struct Vertex pos)
{
    struct Map *map = &state->map;
    for(size_t i = 0; i < map->numVertices; ++i)
    {
        const struct Vertex v = map->vertices[i];
        if(v.x == pos.x && v.y == pos.y) return;
    }

    size_t idx = map->numVertices++;
    map->vertices[idx] = pos;
    if(map->numVertices == map->numAllocVertices)
        IncreaseBufferSize((void**)&map->vertices, &map->numAllocVertices, sizeof *map->vertices);

    ((struct VertexType*)state->gl.editorVertex.bufferMap)[idx] = (struct VertexType){ .position = { pos.x, pos.y }, .color = { 1, 1, 1, 1 } };

    map->dirty = true;
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
    return false;
}

static void IncreaseBufferSize(void **buffer, size_t *capacity, size_t elementSize)
{
    *capacity = (*capacity) * 2;
    *buffer = realloc(*buffer, (*capacity) * elementSize);
}
