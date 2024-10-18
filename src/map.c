#include "map.h"

#include "logging.h"
#include "memory.h" // IWYU pragma: keep
#include <string.h>

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

LineData CopyLineData(LineData data)
{
    LineData copy = data;
    if(data.front.lowerTex)
        copy.front.lowerTex = string_copy(data.front.lowerTex);
    if(data.front.middleTex)
        copy.front.middleTex = string_copy(data.front.middleTex);
    if(data.front.upperTex)
        copy.front.upperTex = string_copy(data.front.upperTex);
    if(data.back.lowerTex)
        copy.back.lowerTex = string_copy(data.back.lowerTex);
    if(data.back.middleTex)
        copy.back.middleTex = string_copy(data.back.middleTex);
    if(data.back.upperTex)
        copy.back.upperTex = string_copy(data.back.upperTex);
    return copy;
}

SectorData CopySectorData(SectorData data)
{
    SectorData copy = data;
    if(data.ceilTex)
        copy.ceilTex = string_copy(data.ceilTex);
    if(data.floorTex)
        copy.floorTex = string_copy(data.floorTex);
    return copy;
}

void FreeLineData(LineData data)
{
    string_free(data.front.lowerTex);
    string_free(data.front.middleTex);
    string_free(data.front.upperTex);
    string_free(data.back.lowerTex);
    string_free(data.back.middleTex);
    string_free(data.back.upperTex);
}

void FreeSectorData(SectorData data)
{
    string_free(data.ceilTex);
    string_free(data.floorTex);
}

void FreeMapVertex(MapVertex *vertex)
{
    free(vertex);
}

void FreeMapLine(MapLine *line)
{
    FreeLineData(line->data);
    free(line);
}

void FreeMapSector(MapSector *sector)
{
    FreeSectorData(sector->data);

    free(sector->outerLines);
    for(size_t i = 0; i < sector->numInnerLines; ++i)
        free(sector->innerLines[i]);
    free(sector->innerLines);

    free(sector->edData.vertices);
    free(sector->edData.indices);

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


    string_free(map->file);
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

static char* getTextureName(pstring texname)
{
    return texname ? texname : "NULL";
}

void SaveMap(Map *map)
{
    if(map->file == NULL) return;

    FILE *saveFile = fopen(map->file, "w");
    if(!saveFile)
    {
        LogError("Failed to create map file: %s", strerror(errno));
        return;
    }
    fprintf(saveFile, "{\n");

    fprintf(saveFile, "\tversion = %d\n", MAP_VERSION);
    fprintf(saveFile, "\teditor = editor2\n");
    fprintf(saveFile, "\tgravity = %f\n", map->gravity);
    fprintf(saveFile, "\ttextureScale = %d\n", map->textureScale);

    fprintf(saveFile, "\tvertices = {\n");
    for(MapVertex *vertex = map->headVertex; vertex; vertex = vertex->next)
    {
        fprintf(saveFile, "\t\t%zu %.4f %.4f\n", vertex->idx, vertex->pos.x, vertex->pos.y);
    }
    fprintf(saveFile, "\t}\n");

    fprintf(saveFile, "\tlines = {\n");
    for(MapLine *line = map->headLine; line; line = line->next)
    {
        fprintf(saveFile, "\t\t%zu %zu %zu %u ", line->idx, line->a->idx, line->b->idx, line->data.type);
        fprintf(saveFile, "%s %s %s ", getTextureName(line->data.front.lowerTex), getTextureName(line->data.front.middleTex), getTextureName(line->data.front.upperTex));
        fprintf(saveFile, "%s %s %s\n", getTextureName(line->data.front.lowerTex), getTextureName(line->data.front.middleTex), getTextureName(line->data.front.upperTex));
    }
    fprintf(saveFile, "\t}\n");

    fprintf(saveFile, "\tsectors = {\n");
    for(MapSector *sector = map->headSector; sector; sector = sector->next)
    {
        fprintf(saveFile, "\t\t%zu %zu ", sector->idx, sector->numOuterLines);
        for(size_t i = 0; i < sector->numOuterLines; ++i)
        {
            fprintf(saveFile, "%zu ", sector->outerLines[i]->idx);
        }
        fprintf(saveFile, "%d %d %u ", sector->data.floorHeight, sector->data.ceilHeight, sector->data.type);
        fprintf(saveFile, "%s %s\n", getTextureName(sector->data.floorTex), getTextureName(sector->data.ceilTex));
    }
    fprintf(saveFile, "\t}\n");

    fprintf(saveFile, "}\n");
    fclose(saveFile);
    map->dirty = false;
}

void FreeMap(Map *map)
{
    FreeVertList(map->headVertex);
    FreeLineList(map->headLine);
    FreeSectorList(map->headSector);

    string_free(map->file);
    map->file = NULL;
}

