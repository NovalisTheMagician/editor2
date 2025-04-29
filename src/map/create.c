#include "create.h"
#include "map.h"

#include <string.h>

CreateResult CreateVertex(Map *map, vec2s pos)
{
    for(MapVertex *vertex = map->headVertex; vertex; vertex = vertex->next)
    {
        if(glms_vec2_eqv_eps(vertex->pos, pos))
        {
            return (CreateResult){ .mapElement = vertex, .created = false };
        }
    }

    MapVertex *vertex = calloc(1, sizeof *vertex);
    vertex->pos = pos;
    vertex->idx = map->vertexIdx++;
    vertex->prev = map->tailVertex;

    if(map->headVertex == NULL)
    {
        map->headVertex = vertex;
        map->tailVertex = vertex;
    }
    else if(map->tailVertex->prev == NULL)
    {
        map->headVertex->next = vertex;
    }
    else
    {
        map->tailVertex->next = vertex;
    }
    map->tailVertex = vertex;
    map->numVertices++;

    map->dirty = true;

    return (CreateResult){ .mapElement = vertex, .created = true };
}

CreateResult CreateLine(Map *map, MapVertex *v0, MapVertex *v1, LineData data)
{
    for(MapLine *line = map->headLine; line; line = line->next)
    {
        bool ab = line->a == v0 && line->b == v1;
        bool ba = line->a == v1 && line->b == v0;
        if(ab || ba)
        {
            return (CreateResult){ .mapElement = line, .created = false };
        }
    }

    MapLine *line = calloc(1, sizeof *line);
    line->a = v0;
    line->b = v1;
    line->idx = map->lineIdx++;
    line->prev = map->tailLine;
    line->data = CopyLineData(data);

    v0->attachedLines[v0->numAttachedLines] = line;
    line->aVertIndex = v0->numAttachedLines;
    v0->numAttachedLines++;
    v1->attachedLines[v1->numAttachedLines] = line;
    line->bVertIndex = v1->numAttachedLines;
    v1->numAttachedLines++;

    if(map->headLine == NULL)
    {
        map->headLine = line;
        map->tailLine = line;
    }
    else if(map->tailLine->prev == NULL)
    {
        map->headLine->next = line;
    }
    else
    {
        map->tailLine->next = line;
    }
    map->tailLine = line;
    map->numLines++;

    map->dirty = true;

    return (CreateResult){ .mapElement = line, .created = true };
}

CreateResult CreateSector(Map *map, size_t numLines, MapLine *lines[static numLines], SectorData data)
{
    for(MapSector *sector = map->headSector; sector; sector = sector->next)
    {
        if(sector->numOuterLines != numLines) continue;

        bool allSame = true;
        for(size_t i = 0; i < numLines; ++i)
        {
            bool sharesALine = false;
            for(size_t j = 0; j < sector->numOuterLines; ++j)
            {
                if(lines[i] == sector->outerLines[j])
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

    MapSector *sector = calloc(1, sizeof *sector);
    sector->numOuterLines = numLines;
    sector->outerLines = malloc(sector->numOuterLines * sizeof *sector->outerLines);
    memcpy(sector->outerLines, lines, sector->numOuterLines * sizeof *sector->outerLines);
    sector->idx = map->sectorIdx++;
    sector->prev = map->tailSector;
    sector->data = CopySectorData(data);

    if(map->headSector == NULL)
    {
        map->headSector = sector;
        map->tailSector = sector;
    }
    else if(map->tailSector->prev == NULL)
    {
        map->headSector->next = sector;
    }
    else
    {
        map->tailSector->next = sector;
    }
    map->tailSector = sector;

    map->numSectors++;

    map->dirty = true;

    return (CreateResult){ .mapElement = sector, .created = true };
}
