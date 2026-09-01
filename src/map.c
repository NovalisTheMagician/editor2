#include "map.h"

#include "edit.h"
#include "logging.h"
#include "map/query.h"
#include "map/insert.h"
#include "serialization.h"
#include "utils/string.h"

#include "mapfile.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <strings.h>

#define KEY_VERSION "version"
#define KEY_EDITOR "editor"
#define KEY_GRAVITY "gravity"
#define KEY_TEXTURESCALE "texture_scale"
#define KEY_VERTICES "vertices"
#define KEY_LINES "lines"
#define KEY_SECTORS "sectors"

#define KEY_IS(k) strcasecmp(key, k) == 0

static void Log(int severity, const char *msg)
{
    switch(severity)
    {
    case MS_INFO: LogInfo(msg); break;
    case MS_WARNING: LogWarning(msg); break;
    case MS_ERROR: LogError(msg); break;
    default: LogError(msg); break;
    }
}

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
    return (SectorData)
    { 
        .type = ST_NORMAL,
        .floorHeight = 0,
        .ceilHeight = 64,
        .lightLevel = 255
    };
}

LineData CopyLineData(LineData data)
{
    LineData copy = data;
    if(data.front.lowerTex)
    {
        copy.front.lowerTex = malloc(strlen(data.front.lowerTex)+1);
        strcpy(copy.front.lowerTex, data.front.lowerTex);
    }
    if(data.front.middleTex)
    {
        copy.front.middleTex = malloc(strlen(data.front.middleTex)+1);
        strcpy(copy.front.middleTex, data.front.middleTex);
    }
    if(data.front.upperTex)
    {
        copy.front.upperTex = malloc(strlen(data.front.upperTex)+1);
        strcpy(copy.front.upperTex, data.front.upperTex);
    }
    if(data.back.lowerTex)
    {
        copy.back.lowerTex = malloc(strlen(data.back.lowerTex)+1);
        strcpy(copy.back.lowerTex, data.back.lowerTex);
    }
    if(data.back.middleTex)
    {
        copy.back.middleTex = malloc(strlen(data.back.middleTex)+1);
        strcpy(copy.back.middleTex, data.back.middleTex);
    }
    if(data.back.upperTex)
    {
        copy.back.upperTex = malloc(strlen(data.back.upperTex)+1);
        strcpy(copy.back.upperTex, data.back.upperTex);
    }
    return copy;
}

SectorData CopySectorData(SectorData data)
{
    SectorData copy = data;
    if(data.ceilTex)
    {
        copy.ceilTex = malloc(strlen(data.ceilTex)+1);
        strcpy(copy.ceilTex, data.ceilTex);
    }
    if(data.floorTex)
    {
        copy.floorTex = malloc(strlen(data.floorTex)+1);
        strcpy(copy.floorTex, data.floorTex);
    }
    return copy;
}

void FreeLineData(LineData data)
{
    free(data.front.lowerTex);
    free(data.front.middleTex);
    free(data.front.upperTex);
    free(data.back.lowerTex);
    free(data.back.middleTex);
    free(data.back.upperTex);
}

void FreeSectorData(SectorData data)
{
    free(data.ceilTex);
    free(data.floorTex);
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
    free(sector->numInnerLinesNum);
    free(sector->innerLines);

    free(sector->edData.vertices);
    free(sector->edData.indices);

    free(sector);
}

void NewMap(Map *map)
{
    FreeVertList(map->headVertex);
    map->headVertex = map->tailVertex = NULL;
    map->numVertices = 0;
    map->vertexIdx = 0;

    FreeLineList(map->headLine);
    map->headLine = map->tailLine = NULL;
    map->numLines = 0;
    map->lineIdx = 0;

    FreeSectorList(map->headSector);
    map->headSector = map->tailSector = NULL;
    map->numSectors = 0;
    map->sectorIdx = 0;

    free(map->file);
    map->file = NULL;

    map->dirty = false;

    map->textureScale = 1;
    map->gravity = 9.80f;
}

static void assignSide(Side *side, MapfileLineSide *lineSide)
{
    side->upperOffset = (Vec2){ lineSide->upperOffsetX, lineSide->upperOffsetY };
    if(lineSide->upperTex)
    {
        side->upperTex = malloc(lineSide->upperTexLen+1);
        memcpy(side->upperTex, lineSide->upperTex, lineSide->upperTexLen);
        side->upperTex[lineSide->upperTexLen] = '\0';
    }

    side->middleOffset = (Vec2){ lineSide->middleOffsetX, lineSide->middleOffsetY };
    if(lineSide->middleTex)
    {
        side->middleTex = malloc(lineSide->middleTexLen+1);
        memcpy(side->middleTex, lineSide->middleTex, lineSide->middleTexLen);
        side->middleTex[lineSide->middleTexLen] = '\0';
    }

    side->lowerOffset = (Vec2){ lineSide->lowerOffsetX, lineSide->lowerOffsetY };
    if(lineSide->lowerTex)
    {
        side->lowerTex = malloc(lineSide->lowerTexLen+1);
        memcpy(side->lowerTex, lineSide->lowerTex, lineSide->lowerTexLen);
        side->lowerTex[lineSide->lowerTexLen] = '\0';
    }
}

bool LoadMap(Map *map, const char *filename)
{
    if(filename == NULL) return false;

    FILE *file = fopen(filename, "rb");
    if(!file)
    {
        LogError("Failed to load map file %s: %s", map->file, strerror(errno));
        return false;
    }

    NewMap(map);

    map->file = CopyString(filename);
    map->vertexIdx = map->lineIdx = map->sectorIdx = 0;

    MapfileSetLogFunc(Log);

    Mapfile mapfile;
    if(!MapfileParse(file, &mapfile, 0))
    {
        fclose(file);
        LogError("Failed to parse mapfile `%s'", map->file);
        return false;
    }

    map->gravity = mapfile.gravity;
    map->textureScale = mapfile.textureScale;

    for(MapfileIterator iterator = MapfileVerticesIterator(&mapfile); !MapfileIteratorEnd(&iterator); MapfileIteratorNextVertex(&iterator))
    {
        MapfileVertex v = { 0 };
        if(MapfileIteratorGetVertex(&iterator, &v))
        {
            MapVertex *vertex = EditAddVertex(map, fvec2_from_vec2((Vec2){ v.x, v.y }));;
            vertex->idx = v.index++;

            if(v.index > map->vertexIdx) map->vertexIdx = v.index;
        }
    }

    for(MapfileIterator iterator = MapfileLinesIterator(&mapfile); !MapfileIteratorEnd(&iterator); MapfileIteratorNextLine(&iterator))
    {
        MapfileLine l = { 0 };
        if(MapfileIteratorGetLine(&iterator, &l))
        {
            LineData data = { 0 };

            assignSide(&data.front, &l.front);
            assignSide(&data.back, &l.back);

            MapVertex *vA = GetVertex(map, l.vertexA);
            if(!vA) continue;
            MapVertex *vB = GetVertex(map, l.vertexB);
            if(!vB) continue;
            MapLine *mapLine = EditAddLine(map, vA, vB, data);
            mapLine->idx = l.index++;

            if(l.index > map->lineIdx) map->lineIdx = l.index;

            FreeLineData(data);
        }
    }

    for(MapfileIterator iterator = MapfileSectorsIterator(&mapfile); !MapfileIteratorEnd(&iterator); MapfileIteratorNextSector(&iterator))
    {
        MapfileSector s = { 0 };
        if(MapfileIteratorGetSector(&iterator, &s))
        {
            SectorData data = { 0 };

            data.floorHeight = s.floorHeight;
            data.ceilHeight = s.ceilHeight;
            data.lightLevel = s.lightLevel;

            data.floorOffset = (Vec2){ s.floorOffsetX, s.floorOffsetY };
            if(s.floorTex)
            {
                data.floorTex = malloc(s.floorTexLen+1);
                memcpy(data.floorTex, s.floorTex, s.floorTexLen);
                data.floorTex[s.floorTexLen] = '\0';
            }

            data.ceilOffset = (Vec2){ s.ceilOffsetX, s.ceilOffsetY };
            if(s.ceilTex)
            {
                data.ceilTex = malloc(s.ceilTexLen+1);
                memcpy(data.ceilTex, s.ceilTex, s.ceilTexLen);
                data.ceilTex[s.ceilTexLen] = '\0';
            }

            MapLine *outerLine = GetLine(map, s.firstLine);
            if(!outerLine)
            {
                LogError("Failed to find line");
                continue;
            }

            MapSector *sector = MakeMapSector(map, outerLine, !s.firstLineFront, data);
            sector->idx = s.index++;
            if(s.index > map->sectorIdx) map->sectorIdx = s.index;

            FreeSectorData(data);
        }
    }

    fclose(file);
    map->dirty = false;
    return true;
}

static char* getTextureName(char *texname)
{
    return texname ? texname : "NULL";
}

static void writeSideTexture(FILE *file, const char *texture, Vec2 offset)
{
    fprintf(file, "%s %.4f %.4f ", texture, offset.x, offset.y);
}

static void writeSide(FILE *file, Side side)
{
    writeSideTexture(file, getTextureName(side.lowerTex), side.lowerOffset);
    writeSideTexture(file, getTextureName(side.middleTex), side.middleOffset);
    writeSideTexture(file, getTextureName(side.upperTex), side.upperOffset);
}

void SaveMap(Map *map)
{
    if(!map->file) return;

    FILE *file = fopen(map->file, "w");
    if(!file)
    {
        LogError("Failed to save map file %s: %s", map->file, strerror(errno));
        return;
    }

    fprintf(file, "version = %d\n", MAP_VERSION);
    fprintf(file, "editor = editor2\n");
    fprintf(file, "gravity = %.4f\n", map->gravity);
    fprintf(file, "textureScale = %d\n", map->textureScale);

    fprintf(file, "vertices = {\n");
    for(MapVertex *vertex = map->headVertex; vertex; vertex = vertex->next)
    {
        fprintf(file, "\t%zu %.4f %.4f\n", vertex->idx, fixed_to_real(vertex->pos.x), fixed_to_real(vertex->pos.y));
    }
    fprintf(file, "}\n");

    fprintf(file, "lines = {\n");
    for(MapLine *line = map->headLine; line; line = line->next)
    {
        fprintf(file, "\t%zu %zu %zu %u ", line->idx, line->a->idx, line->b->idx, line->data.type);
        writeSide(file, line->data.front);
        writeSide(file, line->data.back);
        fprintf(file, "\n");
    }
    fprintf(file, "}\n");

    fprintf(file, "sectors = {\n");
    for(MapSector *sector = map->headSector; sector; sector = sector->next)
    {
        fprintf(file, "\t%zu %zu %d ", sector->idx, sector->outerLines[0]->idx, sector->outerLines[0]->frontSector == sector);
		fprintf(file, "%d %d %u ", sector->data.floorHeight, sector->data.ceilHeight, sector->data.type);
        fprintf(file, "%.4f %.4f ", sector->data.floorOffset.x, sector->data.floorOffset.y);
        fprintf(file, "%.4f %.4f ", sector->data.ceilOffset.x, sector->data.ceilOffset.y);
        fprintf(file, "%u ", sector->data.lightLevel);
        fprintf(file, "%s %s\n", getTextureName(sector->data.floorTex), getTextureName(sector->data.ceilTex));
    }
    fprintf(file, "}\n");

    fclose(file);
    map->dirty = false;
}

void FreeMap(Map *map)
{
    FreeVertList(map->headVertex);
    FreeLineList(map->headLine);
    FreeSectorList(map->headSector);

    free(map->file);
    map->file = NULL;
}

