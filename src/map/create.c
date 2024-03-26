#include "create.h"

struct CreateResult CreateVertex(struct Map map[static 1], vec2s pos)
{
    for(struct MapVertex *vertex = map->headVertex; vertex; vertex = vertex->next)
    {
        if(glms_vec2_eqv_eps(vertex->pos, pos)) 
        {
            return (struct CreateResult){ .mapElement = vertex, .created = false };
        }
    }

    static size_t vertexIndex = 0;

    struct MapVertex *vertex = calloc(1, sizeof *vertex);
    vertex->pos = pos;
    vertex->idx = vertexIndex++;
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

    map->dirty = true;

    return (struct CreateResult){ .mapElement = vertex, .created = true };
}

struct CreateResult CreateLine(struct Map map[static 1], struct MapVertex v0[static 1], struct MapVertex v1[static 1], struct LineData data)
{
    for(struct MapLine *line = map->headLine; line; line = line->next)
    {
        bool ab = line->a == v0 && line->b == v1;
        bool ba = line->a == v1 && line->b == v0;
        if(ab || ba) 
        {
            return (struct CreateResult){ .mapElement = line, .created = false };
        }
    }

    static size_t lineIndex = 0;

    struct MapLine *line = calloc(1, sizeof *line);
    line->a = v0;
    line->b = v1;
    line->idx = lineIndex++;
    line->prev = map->tailLine;
    line->data = data;

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

    map->dirty = true;

    return (struct CreateResult){ .mapElement = line, .created = true };
}

struct CreateResult CreateSector(struct Map map[static 1], size_t numLines, struct MapLine *lines[static numLines], bool lineFronts[static numLines], struct SectorData data)
{
    for(struct MapSector *sector = map->headSector; sector; sector = sector->next)
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
            return (struct CreateResult){ .mapElement = sector, .created = false };
    }

    static size_t sectorIndex = 0;

    struct MapSector *sector = calloc(1, sizeof *sector);
    sector->numOuterLines = numLines;
    sector->outerLines = malloc(sector->numOuterLines * sizeof *sector->outerLines);
    memcpy(sector->outerLines, lines, sector->numOuterLines * sizeof *sector->outerLines);
    sector->idx = sectorIndex++;
    sector->prev = map->tailSector;
    sector->data = data;

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

    map->dirty = true;

    return (struct CreateResult){ .mapElement = sector, .created = true };
}
