#pragma once

#include <stddef.h>
#include "cglm/struct.h"

#define MAP_VERSION 1

typedef enum PropertyType
{
    PROPERTY_STRING,
    PROPERTY_BOOL,
    PROPERTY_NUMBER,
    PROPERTY_ENUM,
} PropertyType;

#define PROPERTY_KEY_MAX_LENGTH 256
#define PROPERTY_VALUE_MAX_LENGTH 512

typedef struct Property
{
    char key[PROPERTY_KEY_MAX_LENGTH], value[PROPERTY_VALUE_MAX_LENGTH];
} Property;

typedef struct PropertyTable
{
    Property *items;
    size_t count, capacity;
} PropertyTable;

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
    vec2s *vertices;
    size_t numVertices;
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

    PropertyTable props;

    struct MapVertex *next, *prev;
} MapVertex;

typedef struct Side
{
    char *upperTex;
    char *middleTex;
    char *lowerTex;
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

    PropertyTable props;

    bool mark, new;

    size_t idx;
    struct MapLine *next, *prev;
} MapLine;

typedef struct SectorData
{
    uint32_t type;

    int32_t floorHeight;
    int32_t ceilHeight;

    char *floorTex;
    char *ceilTex;
} SectorData;

typedef struct MapSector
{
    MapLine **outerLines;
    size_t numOuterLines;

    MapLine ***innerLines;
    size_t *numInnerLinesNum;
    size_t numInnerLines;

    SectorData data;

    BoundingBox bb;

    PropertyTable props;

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
    char *file;

    int textureScale;
    float gravity;

    size_t vertexIdx, lineIdx, sectorIdx;
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
