#include "query.h"
#include "../map.h"
#include "../geometry.h"
#include "arena.h"

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

typedef struct StackEntry
{
    MapVertex *vertex;
    MapVertex *nextVertex;
    MapLine *line;
} StackEntry;

typedef struct Stack
{
    StackEntry *items;
    size_t count, capacity;
} Stack;

typedef struct DeadList
{
    MapLine **items;
    size_t count, capacity;
} DeadList;

static Arena arena = { 0 };

size_t FindLineLoop(MapLine *startLine, MapLine **sectorLines, size_t maxLoopLength, bool reversed, int(*cmpFunc)(const void*, const void*))
{
    assert(maxLoopLength > 0);
    arena_reset(&arena);

    MapVertex *origVertex = reversed ? startLine->b : startLine->a;
    MapVertex *startNext = reversed ? startLine->a : startLine->b;

    Stack stack = { 0 };
    StackEntry entry = { .vertex = origVertex, .nextVertex = startNext, .line = startLine };
    arena_da_append(&arena, &stack, entry);

    DeadList dead = { 0 };

    Path candidates[256];

    for(;;)
    {
        StackEntry top = stack.items[stack.count-1];
        MapVertex *vertex = top.vertex;
        MapVertex *nextVertex = top.nextVertex;
        MapLine *currentLine = top.line;

        size_t numCandidates = 0;
        for(size_t i = 0; i < nextVertex->numAttachedLines && numCandidates < 256; ++i)
        {
            MapLine *attLine = nextVertex->attachedLines[i];
            if(attLine == currentLine)
                continue;

            bool isDead = false;
            for(size_t d = 0; d < dead.count; ++d)
            {
                if(dead.items[d] == attLine)
                {
                    isDead = true;
                    break;
                }
            }
            if(isDead)
                continue;

            MapVertex *otherVertex = nextVertex == attLine->a ? attLine->b : attLine->a;
            float angle = PI2 - AngleOfLines((line_t){ nextVertex->pos, vertex->pos }, (line_t){ nextVertex->pos, otherVertex->pos });
            candidates[numCandidates++] = (Path){ .line = attLine, .nextVertex = otherVertex, .relativeAngle = angle };
        }

        if(numCandidates == 0)
        {
            arena_da_append(&arena, &dead, currentLine);
            //dead[numDead++] = currentLine;
            if(--stack.count == 0)
                return 0;
            continue;
        }

        qsort(candidates, numCandidates, sizeof *candidates, cmpFunc);
        Path chosen = candidates[numCandidates - 1];

        if(nextVertex == origVertex && chosen.line == startLine)
        {
            for(size_t i = 0; i < stack.count; ++i)
                sectorLines[i] = stack.items[i].line;
            return stack.count;
        }

        if(stack.count >= maxLoopLength)
            return 0;

        entry = (StackEntry){ .vertex = nextVertex, .nextVertex = chosen.nextVertex, .line = chosen.line };
        arena_da_append(&arena, &stack, entry);
    }
}
