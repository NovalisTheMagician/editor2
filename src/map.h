#pragma once

#include "common.h"

#define DEFAULT_CAPACITY 256

enum LineType
{
    LT_NORMAL
};

enum SectorType
{
    ST_NORMAL
};

struct Vertex
{
    int32_t x, y;
};

struct MapVertex
{
    struct Vertex pos;

    size_t idx;
    int refCount;
    struct MapVertex *next, *prev;
};

struct Side
{
    pstring upperTex;
    pstring middleTex;
    pstring lowerTex;
};

struct MapLine
{
    struct MapVertex *a, *b;
    uint32_t type;
    
    int32_t normal;

    struct Side front;
    struct Side back;

    size_t idx;
    int refCount;
    struct MapLine *next, *prev;
};

struct MapSector
{
    struct MapLine **outerLines;
    size_t numOuterLines;
    uint32_t type;

    struct MapLine ***innerLines;
    size_t *numInnerLinesNum;
    size_t numInnerLines;

    struct MapSector *containedBy;
    struct MapSector **contains;
    size_t numContains;

    int32_t floorHeight;
    int32_t ceilHeight;

    pstring floorTex;
    pstring ceilTex;

    size_t idx;
    struct MapSector *next, *prev;
};

struct Map
{
    struct MapVertex *headVertex, *tailVertex;
    size_t numVertices;

    struct MapLine *headLine, *tailLine;
    size_t numLines;

    struct MapSector *headSector, *tailSector;
    size_t numSectors;

    bool dirty;
    pstring file;

    int textureScale;
};

void NewMap(struct Map *map);
bool LoadMap(struct Map *map);
void SaveMap(struct Map *map);
void FreeMap(struct Map *map);
