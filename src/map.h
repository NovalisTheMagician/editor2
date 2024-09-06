#pragma once

#include <stddef.h>
#include "cglm/struct.h"
#include "utils/pstring.h"

typedef enum LineType
{
    LT_NORMAL
} LineType;

typedef enum SectorType
{
    ST_NORMAL
} SectorType;

typedef struct TriangleData
{
    vec2s *texCoords;
    uint32_t *indices;
    size_t numIndices;
} TriangleData;

typedef struct BoundingBox
{
    vec2s min, max;
} BoundingBox;

struct MapLine;
struct MapSector;

typedef struct MapVertex
{
    vec2s pos;

    size_t idx;

    struct MapLine *attachedLines[256];
    size_t numAttachedLines;

    struct MapVertex *next, *prev;
} MapVertex;

typedef struct Side
{
    pstring upperTex;
    pstring middleTex;
    pstring lowerTex;
} Side;

typedef struct LineData
{
    Side front, back;
    uint32_t type;
} LineData;

typedef struct MapLine
{
    MapVertex *a, *b;
    size_t aVertIndex, bVertIndex;

    LineData data;
    struct MapSector *frontSector, *backSector;

    size_t idx;
    struct MapLine *next, *prev;
} MapLine;

typedef struct SectorData
{
    uint32_t type;

    int32_t floorHeight;
    int32_t ceilHeight;

    pstring floorTex;
    pstring ceilTex;
} SectorData;

typedef struct MapSector
{
    MapLine **outerLines;
    size_t numOuterLines;
    vec2s *vertices;

    MapLine ***innerLines;
    size_t *numInnerLinesNum;
    size_t numInnerLines;

    struct MapSector *containedBy;
    struct MapSector **contains;
    size_t numContains;

    SectorData data;

    BoundingBox bb;

    size_t idx;
    struct MapSector *next, *prev;
    TriangleData edData;
} MapSector;

typedef struct Map
{
    MapVertex *headVertex, *tailVertex;
    size_t numVertices;

    MapLine *headLine, *tailLine;
    size_t numLines;

    MapSector *headSector, *tailSector;
    size_t numSectors;

    bool dirty;
    pstring file;

    int textureScale;
    float gravity;
} Map;

LineData DefaultLineData(void);
SectorData DefaultSectorData(void);
LineData CopyLineData(LineData data);
SectorData CopySectorData(SectorData data);
void FreeLineData(LineData data);
void FreeSectorData(SectorData data);

void FreeMapVertex(MapVertex *vertex);
void FreeMapLine(MapLine *line);
void FreeMapSector(MapSector *sector);

void NewMap(Map *map);
bool LoadMap(Map *map);
void SaveMap(Map *map);
void FreeMap(Map *map);
