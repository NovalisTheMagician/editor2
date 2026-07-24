#include "query.h"
#include "../map.h"
#include "../geometry.h"

#include <assert.h>
#include <float.h>
#include <stdlib.h>

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

MapVertex* FindClosestVertex(const Map *map, FVec2 pos, fixed_t radius)
{
    MapVertex *closestVertex = NULL;
    fixedw_t closestDistSq = 0, maxDistSq = (fixedw_t)radius * radius;
    for(MapVertex *vertex = map->headVertex; vertex; vertex = vertex->next)
    {
        fixedw_t distSq = fvec2_distance2(vertex->pos, pos);
        if(distSq > maxDistSq) continue;
        if(closestVertex == NULL || distSq < closestDistSq)
        {
            closestDistSq = distSq;
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

size_t FindLineLoop(MapLine *startLine, MapLine **sectorLines, size_t maxLoopLength, bool reversed, int(*cmpFunc)(const void*, const void*))
{
    assert(maxLoopLength > 0);

    MapVertex *origVertex = reversed ? startLine->b : startLine->a;
    sectorLines[0] = startLine;
    size_t numLines = 1;

    MapVertex *vertex = origVertex;
    MapVertex *nextVertex = reversed ? startLine->a : startLine->b;
    MapLine *currentLine = startLine;

    Path candidates[256];

    for(;;)
    {
        size_t numCandidates = 0;
        for(size_t i = 0; i < nextVertex->numAttachedLines && numCandidates < 256; ++i)
        {
            MapLine *attLine = nextVertex->attachedLines[i];
            if(attLine == currentLine)
                continue;

            MapVertex *otherVertex = nextVertex == attLine->a ? attLine->b : attLine->a;
            float angle = PI2 - AngleOfLines((line_t){ nextVertex->pos, vertex->pos }, (line_t){ nextVertex->pos, otherVertex->pos });
            candidates[numCandidates++] = (Path){ .line = attLine, .nextVertex = otherVertex, .relativeAngle = angle };
        }

        Path chosen;
        if(numCandidates == 0)
        {
            chosen = (Path){ .line = currentLine, .nextVertex = vertex };
        }
        else
        {
            qsort(candidates, numCandidates, sizeof *candidates, cmpFunc);
            chosen = candidates[numCandidates - 1];
        }

        if(nextVertex == origVertex && chosen.line == startLine)
            return numLines;

        if(numLines >= maxLoopLength)
            return 0;

        sectorLines[numLines++] = chosen.line;
        vertex = nextVertex;
        nextVertex = chosen.nextVertex;
        currentLine = chosen.line;
    }
}
