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

struct LineData DefaultLineData(void)
{
    return (struct LineData){ .type = LT_NORMAL };
}

struct SectorData DefaultSectorData(void)
{
    return (struct SectorData){ .type = ST_NORMAL };
}

void FreeMapVertex(struct MapVertex *vertex)
{
    free(vertex);
}

void FreeMapLine(struct MapLine *line)
{
    string_free(line->data.front.lowerTex);
    string_free(line->data.front.middleTex);
    string_free(line->data.front.upperTex);
    string_free(line->data.back.lowerTex);
    string_free(line->data.back.middleTex);
    string_free(line->data.back.upperTex);
    free(line);
}

void FreeMapSector(struct MapSector *sector)
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

    if(map->file)
    {
        string_free(map->file);
        map->file = NULL;
    }

    map->dirty = false;

    map->textureScale = 1;
    map->gravity = 9.80f;
}

bool LoadMap(struct Map *map)
{
    map->dirty = false;
    return false;
}

void SaveMap(struct Map *map)
{
    if(map->file == NULL) return;

    map->dirty = false;
}

void FreeMap(struct Map *map)
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

