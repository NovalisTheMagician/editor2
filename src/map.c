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

    pstr_free(map->file);
    map->file = pstr_alloc(0);

    map->dirty = false;

    map->textureScale = 1;
}

bool LoadMap(struct Map *map)
{
    map->dirty = false;
    return false;
}

void SaveMap(struct Map *map)
{
    if(map->file.size == 0) return;

    map->dirty = false;
}
void FreeMap(struct Map *map)
{
    for(size_t i = 0; i < map->numLines; ++i)
    {
        struct Line line = map->lines[i];
        pstr_free(line.front.upperTex);
        pstr_free(line.front.middleTex);
        pstr_free(line.front.lowerTex);
        pstr_free(line.back.upperTex);
        pstr_free(line.back.middleTex);
        pstr_free(line.back.lowerTex);
    }

    for(size_t i = 0; i < map->numSectors; ++i)
    {
        struct Sector sector = map->sectors[i];
        pstr_free(sector.floorTex);
        pstr_free(sector.ceilTex);
    }

    free(map->vertices);
    free(map->lines);
    free(map->sectors);

    pstr_free(map->file);
}

