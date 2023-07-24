#include "map.h"

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
    map->dirty = false;
    return false;
}

void SaveMap(struct Map *map, bool useDialog)
{
    map->dirty = false;
}
