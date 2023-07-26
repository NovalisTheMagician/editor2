#include "map.h"

#include <nfd.h>

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
}

bool LoadMap(struct Map *map)
{
    map->dirty = false;
    return false;
}

void SaveMap(struct Map *map, bool useDialog)
{
    if(useDialog)
    {
        char *newFile = NULL;
        nfdresult_t res = NFD_SaveDialog("map", NULL, &newFile);
        if(res == NFD_OKAY)
        {
            pstr_free(map->file);
            map->file = pstr_cstr(newFile);
            free(newFile);
        }
        else if(res == NFD_CANCEL)
        {
            free(newFile);
            return;
        }
        else
        {
            free(newFile);
            printf("Error: %s\n", NFD_GetError());
            return;
        }
    }
    if(!map->file.size == 0) return;

    map->dirty = false;
}
void FreeMap(struct Map *map)
{
    pstr_free(map->file);
}

