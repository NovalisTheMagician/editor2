#pragma once

#include "common.h"

struct Vertex
{
    int32_t x, y;
};

struct Line
{
    uint32_t a, b;
    uint32_t type;
};

struct Sector
{
    uint32_t *lines;
    uint32_t numLines;
    uint32_t type;
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
    char *file;
};

void NewMap(struct Map *map);
bool LoadMap(struct Map *map, const char *file);
void SaveMap(const struct Map *map, const char *file);

void AddVertex(struct Map *map, struct Vertex pos);
void RemoveVertex(struct Map *map, size_t index);
bool GetVertex(struct Map *map, struct Vertex pos, size_t *ind);
