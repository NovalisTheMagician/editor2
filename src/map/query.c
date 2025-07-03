#include "query.h"
#include "../map.h"
#include "../geometry.h"

#include <assert.h>
#include <float.h>
#include <stdlib.h>

static int vertex_cmp(const void *lhs, const void *rhs)
{
    const MapVertex *l = lhs, *r = rhs;
    if(l->idx < r->idx) return -1;
    else if(l->idx > r->idx) return 1;
    return 0;
}

MapVertex* GetVertex(const Map *map, size_t idx)
{
    return bsearch(&(MapVertex){ .idx = idx }, map->vertexList.items, map->vertexList.count, sizeof *map->vertexList.items, vertex_cmp);
}

static int line_cmp(const void *lhs, const void *rhs)
{
    const MapLine *l = lhs, *r = rhs;
    if(l->idx < r->idx) return -1;
    else if(l->idx > r->idx) return 1;
    return 0;
}

MapLine* GetLine(const Map *map, size_t idx)
{
    return bsearch(&(MapLine){ .idx = idx }, map->lineList.items, map->lineList.count, sizeof *map->lineList.items, line_cmp);
}

static int sector_cmp(const void *lhs, const void *rhs)
{
    const MapSector *l = lhs, *r = rhs;
    if(l->idx < r->idx) return -1;
    else if(l->idx > r->idx) return 1;
    return 0;
}

MapSector* GetSector(const Map *map, size_t idx)
{
    return bsearch(&(MapSector){ .idx = idx }, map->sectorList.items, map->sectorList.count, sizeof *map->sectorList.items, sector_cmp);
}

MapSector* FindEquvivalentSector(const Map *map, size_t numLines, MapLine *lines[static numLines])
{
    for(size_t i = 0; i < map->sectorList.count; ++i)
    {
        MapSector *sector = &map->sectorList.items[i];
        if(sector->numOuterLines != numLines) continue;

        bool allSame = true;
        for(size_t i = 0; i < numLines; ++i)
        {
            bool sharesALine = false;
            for(size_t j = 0; j < sector->numOuterLines; ++j)
            {
                if(lines[i]->idx == sector->outerLines[j])
                {
                    sharesALine = true;
                    break;
                }
            }
            if(!sharesALine)
            {
                allSame = false;
                break;
            }
        }

        if(allSame)
            return sector;
    }
    return NULL;
}

MapVertex* FindClosestVertex(const Map *map, vec2s position, float radiusSq)
{
    float closestDist = FLT_MAX;
    MapVertex *closestVertex = NULL;
    for(size_t i = 0; i < map->vertexList.count; ++i)
    {
        MapVertex *vertex = &map->vertexList.items[i];
        float distSq = glms_vec2_distance2(vertex->pos, position);
        if(distSq <= radiusSq && distSq < closestDist)
        {
            closestDist = distSq;
            closestVertex = vertex;
        }
    }
    return closestVertex;
}

typedef struct Path
{
    MapLine *line;
    MapVertex *nextVertex;
    float relativeAngle;
} Path;

typedef struct PathStack
{
    Path paths[100];
    size_t numPaths;
    MapVertex *vertex;
} PathStack;

int angleSortOuter(const void *a, const void *b)
{
    const Path *aPath = a;
    const Path *bPath = b;
    if(aPath->relativeAngle > bPath->relativeAngle) return -1;
    if(aPath->relativeAngle < bPath->relativeAngle) return 1;
    return 0;
}

int angleSortInner(const void *a, const void *b)
{
    const Path *aPath = a;
    const Path *bPath = b;
    if(aPath->relativeAngle < bPath->relativeAngle) return -1;
    if(aPath->relativeAngle > bPath->relativeAngle) return 1;
    return 0;
}

static void insertPath(Map *map, PathStack *pathStack, MapLine *line, MapVertex *nextVertex, MapVertex *vertex, int(*cmpFunc)(const void*, const void*))
{
    pathStack->vertex = nextVertex;
    for(size_t i = 0; i < nextVertex->numAttachedLines; ++i)
    {
        MapLine *attLine = GetLine(map, nextVertex->attachedLines[i]);
        if(attLine->idx == line->idx) continue;

        MapVertex *otherVertex = GetVertex(map, nextVertex->idx == attLine->a ? attLine->b : attLine->a);
        float angle = PI2 - AngleOfLines((line_t){ .a = nextVertex->pos, .b = vertex->pos }, (line_t){ .a = nextVertex->pos, .b = otherVertex->pos });

        Path *path = &pathStack->paths[pathStack->numPaths++];
        path->line = attLine;
        path->relativeAngle = angle;
        path->nextVertex = GetVertex(map, attLine->a == nextVertex->idx ? attLine->b : attLine->a);
    }
    qsort(pathStack->paths, pathStack->numPaths, sizeof *pathStack->paths, cmpFunc);
}

size_t FindLineLoop(Map *map, MapLine *startLine, MapLine **sectorLines, size_t maxLoopLength, int(*cmpFunc)(const void*, const void*))
{
    assert(maxLoopLength > 0);

    // front means natural direction
    sectorLines[0] = startLine;
    size_t numLines = 1;

    MapVertex *mapVertexForNext = GetVertex(map, startLine->b), *mapVertex = GetVertex(map, startLine->a);
    PathStack *pathStack = calloc(1024, sizeof *pathStack);
    size_t pathTop = 0;
    insertPath(map, &pathStack[pathTop++], startLine, mapVertexForNext, mapVertex, cmpFunc);

    bool foundLoop = false;
    while(pathTop > 0)
    {
        PathStack *pathElement = &pathStack[pathTop-1];

        // dead-end, go back
        if(pathElement->numPaths == 0)
        {
            pathTop--;
            numLines--;
            continue;
        }

        Path path = pathElement->paths[--pathElement->numPaths];
        MapLine *mapLine = path.line;

        // found a loop
        if(mapLine == startLine)
        {
            foundLoop = true;
            break;
        }

        // add current line to list
        size_t idx = numLines++;
        sectorLines[idx] = mapLine;

        PathStack *stackPush = &pathStack[pathTop++];
        insertPath(map, stackPush, mapLine, path.nextVertex, pathElement->vertex, cmpFunc);
    }

    free(pathStack);

    if(!foundLoop) numLines = 0;
    return numLines;
}
