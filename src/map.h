#pragma once

#include "common.h"

enum LineType
{
    LT_NORMAL
};

enum SectorType
{
    ST_NORMAL
};

struct TriangleData
{
    size_t indexStart, indexLength;
    size_t vertexStart, vertexLength;
};

struct BoundingBox
{
    vec2s min, max;
};

struct MapLine;
struct MapSector;

struct MapVertex
{
    vec2s pos;

    size_t idx;

    struct MapLine *attachedLines[256];
    size_t numAttachedLines;

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
    size_t aVertIndex, bVertIndex;
    uint32_t type;
    
    int32_t normal;

    struct Side front, back;

    struct MapSector *frontSector, *backSector;

    size_t idx;
    struct MapLine *next, *prev;
};

struct MapSector
{
    struct MapLine **outerLines;
    size_t numOuterLines;
    vec2s *vertices;
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

    struct BoundingBox bb;

    size_t idx;
    struct MapSector *next, *prev;
    struct TriangleData edData;
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
