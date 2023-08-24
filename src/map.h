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

struct Side
{
    pstring upperTex;
    pstring middleTex;
    pstring lowerTex;
};

struct Line
{
    uint32_t a, b;
    uint32_t type;
    
    int32_t normal;

    struct Side front;
    struct Side back;
};

struct Sector
{
    uint32_t *lines;
    uint32_t numLines;
    uint32_t type;

    int32_t floorHeight;
    int32_t ceilHeight;

    pstring floorTex;
    pstring ceilTex;
};

struct Map
{
    struct Vertex *vertices;
    size_t numVertices, numAllocVertices;

    struct Line *lines;
    size_t numLines, numAllocLines;

    struct Sector *sectors;
    size_t numSectors, numAllocSectors;

    bool dirty;
    pstring file;

    int textureScale;
};

void NewMap(struct Map *map);
bool LoadMap(struct Map *map);
void SaveMap(struct Map *map);
void FreeMap(struct Map *map);
