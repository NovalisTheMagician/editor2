#include "create.h"
#include "map.h"

#include <string.h>

#define da_append(da, item) \
do {\
    if(da.count >= da.capacity) {\
        if(da.capacity == 0) da.capacity = INITIAL_MAP_LIST_CAPACITY;\
        else da.capacity *= 2;\
        da.items = realloc(da.items, da.capacity*sizeof *da.items);\
    }\
    da.items[da.count++] = item;\
} while(0)

CreateResult CreateVertex(Map *map, vec2s pos)
{
    for(size_t i = 0; i < map->vertexList.count; ++i)
    {
        MapVertex *vertex = &map->vertexList.items[i];
        if(glms_vec2_eqv_eps(vertex->pos, pos))
        {
            return (CreateResult){ .mapElement = vertex, .created = false };
        }
    }

    MapVertex vertex = { .idx = map->vertexIdx++, .pos = pos };
    da_append(map->vertexList, vertex);

    map->dirty = true;

    return (CreateResult){ .mapElement = &map->vertexList.items[map->vertexList.count-1], .created = true };
}

CreateResult CreateLine(Map *map, MapVertex *v0, MapVertex *v1, LineData data)
{
    for(size_t i = 0; i < map->lineList.count; ++i)
    {
        MapLine *line = &map->lineList.items[i];
        bool ab = line->a == v0->idx && line->b == v1->idx;
        bool ba = line->a == v1->idx && line->b == v0->idx;
        if(ab || ba)
        {
            return (CreateResult){ .mapElement = line, .created = false };
        }
    }

    MapLine line = { .idx = map->lineIdx++, .data = CopyLineData(data), .a = v0->idx, .b = v1->idx, .frontSector = -1, .backSector = -1 };
    v0->attachedLines[v0->numAttachedLines] = line.idx;
    line.aVertIndex = v0->numAttachedLines;
    v0->numAttachedLines++;
    v1->attachedLines[v1->numAttachedLines] = line.idx;
    line.bVertIndex = v1->numAttachedLines;
    v1->numAttachedLines++;

    da_append(map->lineList, line);

    map->dirty = true;

    return (CreateResult){ .mapElement = &map->lineList.items[map->lineList.count-1], .created = true };
}

CreateResult CreateSector(Map *map, size_t numLines, MapLine *lines[static numLines], SectorData data)
{
    for(size_t i = 0; i < map->sectorList.count; ++i)
    {
        MapSector *sector = &map->sectorList.items[i];
        if(sector->numOuterLines != numLines) continue;

        bool allSame = true;
        for(size_t i = 0; i < numLines; ++i)
        {
            bool sharesALine = false;
            for(size_t j = 0; j < sector->numOuterLines; ++j)
            {
                if(lines[i]->idx == sector->outerLines[j])
                {
                    sharesALine = true;
                    break;
                }
            }
            if(!sharesALine)
            {
                allSame = false;
                break;
            }
        }

        if(allSame)
            return (CreateResult){ .mapElement = sector, .created = false };
    }

    MapSector sector = { .idx = map->sectorIdx++, .numOuterLines = numLines, .data = CopySectorData(data), .outerLines = malloc(numLines * sizeof *sector.outerLines) };
    for(size_t i = 0; i < numLines; ++i)
        sector.outerLines[i] = lines[i]->idx;

    da_append(map->sectorList, sector);

    map->dirty = true;

    return (CreateResult){ .mapElement = &map->sectorList.items[map->sectorList.count - 1], .created = true };
}
