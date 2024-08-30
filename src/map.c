#include "map.h"

static void FreeVertList(MapVertex *head)
{
    while(head)
    {
        MapVertex *vertex = head;
        head = head->next;

        FreeMapVertex(vertex);
    }
}

static void FreeLineList(MapLine *head)
{
    while(head)
    {
        MapLine *line = head;
        head = head->next;

        FreeMapLine(line);
    }
}

static void FreeSectorList(MapSector *head)
{
    while(head)
    {
        MapSector *sector = head;
        head = head->next;

        FreeMapSector(sector);
    }
}

LineData DefaultLineData(void)
{
    return (LineData){ .type = LT_NORMAL };
}

SectorData DefaultSectorData(void)
{
    return (SectorData){ .type = ST_NORMAL };
}

void FreeMapVertex(MapVertex *vertex)
{
    free(vertex);
}

void FreeMapLine(MapLine *line)
{
    string_free(line->data.front.lowerTex);
    string_free(line->data.front.middleTex);
    string_free(line->data.front.upperTex);
    string_free(line->data.back.lowerTex);
    string_free(line->data.back.middleTex);
    string_free(line->data.back.upperTex);
    free(line);
}

void FreeMapSector(MapSector *sector)
{
    string_free(sector->data.ceilTex);
    string_free(sector->data.floorTex);
    free(sector->vertices);

    free(sector->outerLines);
    for(size_t i = 0; i < sector->numInnerLines; ++i)
        free(sector->innerLines[i]);
    free(sector->innerLines);

    free(sector->contains);

    free(sector);
}

void NewMap(Map *map)
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

    if(map->file)
    {
        string_free(map->file);
    }
    map->file = string_alloc(1);

    map->dirty = false;

    map->textureScale = 1;
    map->gravity = 9.80f;
}

bool LoadMap(Map *map)
{
    map->dirty = false;
    return false;
}

void SaveMap(Map *map)
{
    if(map->file == NULL) return;

    map->dirty = false;
}

void FreeMap(Map *map)
{
    FreeVertList(map->headVertex);
    FreeLineList(map->headLine);
    FreeSectorList(map->headSector);

    if(map->file)
    {
        string_free(map->file);
        map->file = NULL;
    }
}

