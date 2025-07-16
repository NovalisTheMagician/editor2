#include "query.h"
#include "../map.h"
#include "../geometry.h"

#include <assert.h>

MapVertex* GetVertex(Map *map, size_t idx)
{
    for(MapVertex *vertex = map->headVertex; vertex; vertex = vertex->next)
        if(vertex->idx == idx)
            return vertex;
    return NULL;
}

MapLine* GetLine(Map *map, size_t idx)
{
    for(MapLine *line = map->headLine; line; line = line->next)
        if(line->idx == idx)
            return line;
    return NULL;
}

MapSector* GetSector(Map *map, size_t idx)
{
    for(MapSector *sector = map->headSector; sector; sector = sector->next)
        if(sector->idx == idx)
            return sector;
    return NULL;
}

MapSector* FindEquivalentSector(Map *map, size_t numLines, MapLine *lines[static numLines])
{
    for(MapSector *sector = map->headSector; sector; sector = sector->next)
    {
        if(sector->numOuterLines != numLines) continue;

        bool allSame = true;
        for(size_t i = 0; i < numLines; ++i)
        {
            bool sharesALine = false;
            for(size_t j = 0; j < sector->numOuterLines; ++j)
            {
                if(lines[i] == sector->outerLines[j])
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

static void insertPath(PathStack *pathStack, MapLine *line, MapVertex *nextVertex, MapVertex *vertex, int(*cmpFunc)(const void*, const void*))
{
    pathStack->vertex = nextVertex;
    for(size_t i = 0; i < nextVertex->numAttachedLines; ++i)
    {
        MapLine *attLine = nextVertex->attachedLines[i];
        if(attLine == line) continue;

        MapVertex *otherVertex = nextVertex == attLine->a ? attLine->b : attLine->a;
        float angle = PI2 - AngleOfLines((line_t){ .a = nextVertex->pos, .b = vertex->pos }, (line_t){ .a = nextVertex->pos, .b = otherVertex->pos });

        Path *path = &pathStack->paths[pathStack->numPaths++];
        path->line = attLine;
        path->relativeAngle = angle;
        path->nextVertex = attLine->a == nextVertex ? attLine->b : attLine->a;
    }
    qsort(pathStack->paths, pathStack->numPaths, sizeof *pathStack->paths, cmpFunc);
}

size_t FindLineLoop(MapLine *startLine, MapLine **sectorLines, size_t maxLoopLength, int(*cmpFunc)(const void*, const void*))
{
    assert(maxLoopLength > 0);

    // front means natural direction
    sectorLines[0] = startLine;
    size_t numLines = 1;

    MapVertex *mapVertexForNext = startLine->b, *mapVertex = startLine->a;
    PathStack *pathStack = calloc(1024, sizeof *pathStack);
    size_t pathTop = 0;
    insertPath(&pathStack[pathTop++], startLine, mapVertexForNext, mapVertex, cmpFunc);

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
        insertPath(stackPush, mapLine, path.nextVertex, pathElement->vertex, cmpFunc);
    }

    free(pathStack);

    if(!foundLoop) numLines = 0;
    return numLines;
}
