#include "map.h"

#define DEFAULT_CAPACITY 256

static void IncreaseBufferSize(void **buffer, size_t *capacity, size_t elementSize);

void NewMap(struct Map *map)
{
    if(map->vertices) free(map->vertices);
    map->vertices = calloc(DEFAULT_CAPACITY, sizeof *map->vertices);
    map->numVertices = 0;
    map->numAllocVertices = DEFAULT_CAPACITY;

    if(map->lines) free(map->lines);
    map->lines = calloc(DEFAULT_CAPACITY, sizeof *map->lines);
    map->numLines = 0;
    map->numAllocLines = DEFAULT_CAPACITY;

    if(map->sectors) free(map->sectors);
    map->sectors = calloc(DEFAULT_CAPACITY, sizeof *map->sectors);
    map->numSectors = 0;
    map->numAllocSectors = DEFAULT_CAPACITY;

    if(map->file) free(map->file);
    map->file = NULL;

    map->dirty = false;
}

bool LoadMap(struct Map *map)
{
    return false;
}

void SaveMap(const struct Map *map, bool useDialog)
{

}


void AddVertex(struct Map *map, struct Vertex pos)
{
    for(size_t i = 0; i < map->numVertices; ++i)
    {
        const struct Vertex v = map->vertices[i];
        if(v.x == pos.x && v.y == pos.y) return;
    }

    map->vertices[map->numVertices++] = pos;
    if(map->numVertices == map->numAllocVertices)
        IncreaseBufferSize((void**)&map->vertices, &map->numAllocVertices, sizeof *map->vertices);
}

void RemoveVertex(struct Map *map, size_t index)
{
    if(index >= map->numVertices) return;

    // shift everything above index down
    for(size_t i = index + 1; i < map->numVertices; ++i)
    {
        map->vertices[i-1] = map->vertices[i];
    }
    map->numVertices--;

    // remove lines affected by vertex
}

bool GetVertex(struct Map *map, struct Vertex pos, size_t *ind)
{
    return false;
}

static void IncreaseBufferSize(void **buffer, size_t *capacity, size_t elementSize)
{
    *capacity = (*capacity) * 2;
    *buffer = realloc(*buffer, (*capacity) * elementSize);
}
