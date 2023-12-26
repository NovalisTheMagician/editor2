#include "map.h"

static void FreeVertList(struct MapVertex *head)
{
    while(head)
    {
        struct MapVertex *vertex = head;
        head = head->next;

        FreeMapVertex(vertex);
    }
}

static void FreeLineList(struct MapLine *head)
{
    while(head)
    {
        struct MapLine *line = head;
        head = head->next;

        FreeMapLine(line);
    }
}

static void FreeSectorList(struct MapSector *head)
{
    while(head)
    {
        struct MapSector *sector = head;
        head = head->next;

        FreeMapSector(sector);
    }
}

void FreeMapVertex(struct MapVertex *vertex)
{
    free(vertex);
}

void FreeMapLine(struct MapLine *line)
{
    pstr_free(line->front.lowerTex);
    pstr_free(line->front.middleTex);
    pstr_free(line->front.upperTex);
    pstr_free(line->back.lowerTex);
    pstr_free(line->back.middleTex);
    pstr_free(line->back.upperTex);
    free(line);
}

void FreeMapSector(struct MapSector *sector)
{
    pstr_free(sector->ceilTex);
    pstr_free(sector->floorTex);
    free(sector->vertices);

    free(sector->outerLines);
    for(size_t i = 0; i < sector->numInnerLines; ++i)
        free(sector->innerLines[i]);
    free(sector->innerLines);

    free(sector->contains);

    free(sector);
}

void NewMap(struct Map *map)
{
    FreeVertList(map->headVertex);
    map->headVertex = map->tailVertex = NULL;
    map->numVertices = 0;

    FreeLineList(map->headLine);
    map->headLine = map->tailLine = NULL;
    map->numLines = 0;

    FreeSectorList(map->headSector);
    map->headSector = map->tailSector = NULL;
    map->numSectors = 0;

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
    FreeVertList(map->headVertex);
    FreeLineList(map->headLine);
    FreeSectorList(map->headSector);

    pstr_free(map->file);
}

