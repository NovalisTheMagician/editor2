#include "insert.h"

#include <assert.h>
#include <stdlib.h>

#include "arena.h"

#include "../edit.h"
#include "../geometry.h"
#include "../map.h"
#include "logging.h"
#include "remove.h"
#include "triangulate.h"
#include "util.h"
#include "query.h"
#include "utils.h"

#define MAX_LINES_PER_SECTOR 1024
#define STITCHING_DIST 8.0f

static bool includes(size_t num, void *elements[static num], void *v)
{
    for(size_t i = 0; i < num; ++i)
    {
        if(elements[i] == v)
            return true;
    }
    return false;
}

static void insert(size_t size, size_t *num, void *elements[static size], void *element)
{
    if(!includes(*num, elements, element))
    {
        assert(*num+1 < size);
        elements[(*num)++] = element;
    }
}

MapSector* MakeMapSector(Map *map, MapLine *startLine, SectorData data)
{
    MapLine *sectorLines[MAX_LINES_PER_SECTOR] = { 0 };
    size_t numLines = FindOuterLineLoop(startLine, sectorLines, MAX_LINES_PER_SECTOR);
    if(numLines == 0) return NULL;
    if(FindEquivalentSector(map, numLines, sectorLines)) return NULL;
    struct Polygon *poly = PolygonFromMapLines(numLines, sectorLines);

    Arena arena = { 0 };

    size_t numInnerLineLoops = 0, sizeInnerLineLoops = MAX_LINES_PER_SECTOR, usedLinesTop = 0, usedLinesSize = 4096, numPotentialLines = 0, sizePotentialLines = 1024;
    MapLine ***innerLines = arena_alloc(&arena, sizeInnerLineLoops * sizeof *innerLines);
    size_t *innerLinesNum = arena_alloc(&arena, sizeInnerLineLoops * sizeof *innerLinesNum);
    MapLine **usedLines = arena_alloc(&arena, usedLinesSize * sizeof *usedLines);
    MapLine **potentialLines = arena_alloc(&arena, sizePotentialLines * sizeof *potentialLines);

    for(MapLine *line = map->headLine; line; line = line->next)
    {
        if(includes(numLines, (void**)sectorLines, line))
            continue;

        bool aIn = PointInPolygon(poly, line->a->pos);
        bool bIn = PointInPolygon(poly, line->b->pos);
        if(aIn && bIn)
            potentialLines[numPotentialLines++] = line;
    }

    if(numPotentialLines >= 3) // need at least 3 lines to form a sector
    {
        while(numPotentialLines > 0)
        {
            size_t id = numInnerLineLoops++;
            Arena_Mark mark = arena_snapshot(&arena);
            innerLines[id] = arena_alloc(&arena, MAX_LINES_PER_SECTOR * sizeof **innerLines);
            MapLine *potentialLine = potentialLines[numPotentialLines-1];
            if(!potentialLine) goto nextIteration;
            size_t n = FindInnerLineLoop(potentialLine, innerLines[id], MAX_LINES_PER_SECTOR);
            if(n == 0) // couldnt find a loop
            {
                numInnerLineLoops--;
                insert(usedLinesSize, &usedLinesTop, (void**)usedLines, potentialLine);
            }
            else
            {
                for(size_t i = 0; i < n; ++i)
                {
                    MapLine *line = innerLines[id][i];
                    if(includes(usedLinesTop, (void**)usedLines, line))
                    {
                        numInnerLineLoops--;
                        goto nextIteration;
                    }
                    else
                        insert(usedLinesSize, &usedLinesTop, (void**)usedLines, potentialLine);
                }
                innerLinesNum[id] = n;
            }
    nextIteration:
            arena_rewind(&arena, mark);
            numPotentialLines--;
        }
    }

    free(poly);
    MapSector *sector = EditAddSector(map, numLines, sectorLines, numInnerLineLoops, innerLinesNum, innerLines, data);

    arena_free(&arena);

    return sector;
}

#define QUEUE_SIZE (4096)

typedef struct QueueElement
{
    line_t line;
    bool potentialStart;
} QueueElement;

typedef struct LineQueue
{
    QueueElement elements[QUEUE_SIZE];
    size_t head, tail, numLines;
} LineQueue;

static inline bool Enqueue(LineQueue *queue, line_t line, bool potentialStart)
{
    queue->elements[queue->tail] = (QueueElement){ .line = line, .potentialStart = potentialStart };
    queue->tail = (queue->tail + 1) % QUEUE_SIZE;
    queue->numLines++;
    //assert(queue->numLines < QUEUE_SIZE);
    return queue->numLines < QUEUE_SIZE;
}

static inline QueueElement Dequeue(LineQueue *queue)
{
    QueueElement el = queue->elements[queue->head];
    queue->head = (queue->head + 1) % QUEUE_SIZE;
    queue->numLines--;
    return el;
}

typedef struct SectorUpdateItem
{
    MapLine *line;
    SectorData sectorData;
} SectorUpdateItem;

typedef struct SectorUpdate
{
    SectorUpdateItem *items;
    size_t count, capacity;
} SectorUpdate;

static Arena sectorUpdateArena = { 0 };

static inline void InsertSectorUpdate(SectorUpdate *sectorUpdate, MapLine *line, SectorData sectorData)
{
    SectorUpdateItem item = { .line = line, .sectorData = sectorData };
    arena_da_append(&sectorUpdateArena, sectorUpdate, item);
}

static void DoSplit(Map *map, SectorUpdate *sectorUpdate, MapLine *line, MapVertex *vertex)
{
    SectorData frontData = DefaultSectorData();
    SectorData backData = DefaultSectorData();
    bool hasFrontSector = line->frontSector != NULL;
    bool hasBackSector = line->backSector != NULL;
    bool hasSectorsAttached = hasFrontSector || hasBackSector;

    if(hasFrontSector)
    {
        frontData = CopySectorData(line->frontSector->data);
        MapSector *sector = line->frontSector;
        for(size_t i = 0; i < sector->numOuterLines; ++i)
        {
            MapLine *sline = sector->outerLines[i];
            if(sline == line) continue;
            sline->mark = true;
            InsertSectorUpdate(sectorUpdate, sline, frontData);
        }
        RemoveSector(map, sector);
    }
    if(hasBackSector)
    {
        backData = CopySectorData(line->backSector->data);
        MapSector *sector = line->backSector;
        for(size_t i = 0; i < sector->numOuterLines; ++i)
        {
            MapLine *sline = sector->outerLines[i];
            if(sline == line) continue;
            sline->mark = true;
            InsertSectorUpdate(sectorUpdate, sline, backData);
        }
        RemoveSector(map, sector);
    }

    SplitResult result = SplitMapLine(map, line, vertex);
    if(hasSectorsAttached)
    {
        result.left->mark = true;
        result.right->mark = true;
        if(hasFrontSector)
        {
            InsertSectorUpdate(sectorUpdate, result.left, frontData);
            InsertSectorUpdate(sectorUpdate, result.right, frontData);
        }
        if(hasBackSector)
        {
            InsertSectorUpdate(sectorUpdate, result.left, backData);
            InsertSectorUpdate(sectorUpdate, result.right, backData);
        }
    }
}

static void DoSplit2(Map *map, SectorUpdate *sectorUpdate, MapLine *line, MapVertex *vertexA, MapVertex *vertexB)
{
    SectorData frontData = DefaultSectorData();
    SectorData backData = DefaultSectorData();
    bool hasFrontSector = line->frontSector != NULL;
    bool hasBackSector = line->backSector != NULL;
    bool hasSectorsAttached = hasFrontSector || hasBackSector;

    if(hasFrontSector)
    {
        frontData = CopySectorData(line->frontSector->data);
        MapSector *sector = line->frontSector;
        for(size_t i = 0; i < sector->numOuterLines; ++i)
        {
            MapLine *sline = sector->outerLines[i];
            if(sline == line) continue;
            sline->mark = true;
            InsertSectorUpdate(sectorUpdate, sline, frontData);
        }
        RemoveSector(map, sector);
    }
    if(hasBackSector)
    {
        backData = CopySectorData(line->backSector->data);
        MapSector *sector = line->backSector;
        for(size_t i = 0; i < sector->numOuterLines; ++i)
        {
            MapLine *sline = sector->outerLines[i];
            if(sline == line) continue;
            sline->mark = true;
            InsertSectorUpdate(sectorUpdate, sline, backData);
        }
        RemoveSector(map, sector);
    }

    SplitResult result = SplitMapLine2(map, line, vertexA, vertexB);
    if(hasSectorsAttached)
    {
        result.left->mark = true;
        result.right->mark = true;
        result.middle->mark = true;
        if(hasFrontSector)
        {
            InsertSectorUpdate(sectorUpdate, result.left, frontData);
            InsertSectorUpdate(sectorUpdate, result.right, frontData);
            InsertSectorUpdate(sectorUpdate, result.middle, frontData);
        }
        if(hasBackSector)
        {
            InsertSectorUpdate(sectorUpdate, result.left, backData);
            InsertSectorUpdate(sectorUpdate, result.right, backData);
            InsertSectorUpdate(sectorUpdate, result.middle, backData);
        }
    }
}

bool InsertLinesIntoMap(Map *map, size_t numVerts, Vec2 vertices[static numVerts], bool isLoop)
{
    bool didIntersect = false;
    size_t end = isLoop ? numVerts : numVerts - 1;

    if(isLoop)
    {
        enum orientation_t orientation = LineLoopOrientation(numVerts, vertices);
        if(orientation == CCW_ORIENT)
        {
            size_t half = numVerts / 2;
            for(size_t i = 0; i < half; ++i)
            {
                Vec2 tmp = vertices[i];
                vertices[i] = vertices[numVerts - i - 1];
                vertices[numVerts - i - 1] = tmp;
            }
        }
    }

    LineQueue queue = { 0 };
    size_t numNewLines = 0;

    SectorUpdate sectorsToUpdate = { 0 };

    // insert the drawn lines into queue
    for(size_t i = 0; i < end; ++i)
    {
        Vec2 a = vertices[i];
        Vec2 b = vertices[(i+1) % numVerts];

        if(!Enqueue(&queue, (line_t){ .a = a, .b = b }, i == 0)) return false;
    }

    LogDebug("Start inserting...");
    while(queue.numLines > 0)
    {
        QueueElement el = Dequeue(&queue);
        LogDebug("Remove 1 (%zu)", queue.numLines);
        line_t line = el.line;
        if(eq(vec2_distance2(line.a, line.b), 0)) continue;
        bool potentialStart = el.potentialStart;

        bool canInsertLine = true;
        MapLine *mapLine = map->headLine;
        while(mapLine != NULL)
        {
            line_t mline = { .a = mapLine->a->pos, .b = mapLine->b->pos };

            intersection_res_t intersection = { 0 };
            if(LineOverlap(mline, line, &intersection))
            {
                double u0 = intersection.u, u1 = intersection.v;
                LogDebug("Overlap: U0: %.32f, U1: %.32f", u0, u1);
                if((eq(u0, 0) && eq(u1, 1)) || (eq(u0, 1) && eq(u1, 0))) // lines are equal
                {
                    LogDebug("-> equal lines");
                    mapLine = NULL;
                }
                else if(eq(u0, 0))
                {
                    if(gt(u1, 1))
                    {
                        LogDebug("-> start at end point and end outside");
                        line_t line1 = { mline.b, line.b };
                        if(!Enqueue(&queue, line1, true)) return false;
                        LogDebug("Add 1 %s(%s:%d)", __FUNCTION__, __FILE__, __LINE__);
                    }
                    else if(lt(u1, 1))
                    {
                        LogDebug("-> start at end point and end inside");
                        MapVertex *splitVertex = EditAddVertex(map, intersection.p1);
                        DoSplit(map, &sectorsToUpdate, mapLine, splitVertex);
                    }
                    mapLine = NULL;
                }
                else if(eq(u0, 1))
                {
                    if(lt(u1, 0))
                    {
                        LogDebug("-> start at end point and end outside reverse");
                        line_t line1 = { mline.a, line.b };
                        if(!Enqueue(&queue, line1, true)) return false;
                        LogDebug("Add 1 %s(%s:%d)", __FUNCTION__, __FILE__, __LINE__);
                    }
                    else if(gt(u1, 0))
                    {
                        LogDebug("-> start at end point and end inside reverse");
                        MapVertex *splitVertex = EditAddVertex(map, intersection.p1);
                        DoSplit(map, &sectorsToUpdate, mapLine, splitVertex);
                    }
                    mapLine = NULL;
                }
                else if(eq(u1, 0))
                {
                    if(gt(u0, 1))
                    {
                        LogDebug("-> start outside and end at endpoint reverse");
                        line_t line1 = { line.a, mline.b };
                        if(!Enqueue(&queue, line1, true)) return false;
                        LogDebug("Add 1 %s(%s:%d)", __FUNCTION__, __FILE__, __LINE__);
                    }
                    else if(lt(u0, 1))
                    {
                        LogDebug("-> start inside and end on endpoint reverse");
                        MapVertex *splitVertex = EditAddVertex(map, intersection.p0);
                        DoSplit(map, &sectorsToUpdate, mapLine, splitVertex);
                    }
                    mapLine = NULL;
                }
                else if(eq(u1, 1))
                {
                    if(lt(u0, 0))
                    {
                        LogDebug("-> start outside and end at endpoint");
                        line_t line1 = { line.a, mline.a };
                        if(!Enqueue(&queue, line1, true)) return false;
                        LogDebug("Add 1 %s(%s:%d)", __FUNCTION__, __FILE__, __LINE__);
                    }
                    else if(gt(u0, 0))
                    {
                        LogDebug("-> start inside and end on endpoint");
                        MapVertex *splitVertex = EditAddVertex(map, intersection.p0);
                        DoSplit(map, &sectorsToUpdate, mapLine, splitVertex);
                    }
                    mapLine = NULL;
                }
                else if(gt(u0, 0) && lt(u0, 1) && gt(u1, 0) && lt(u1, 1)) // start and end inside
                {
                    LogDebug("-> start and end inside");
                    MapVertex *splitVertex1 = EditAddVertex(map, intersection.p0);
                    MapVertex *splitVertex2 = EditAddVertex(map, intersection.p1);
                    DoSplit2(map, &sectorsToUpdate, mapLine, splitVertex1, splitVertex2);
                    mapLine = NULL;
                }
                else if(lt(u0, 0) && lt(u1, 1)) // start outside and end inside
                {
                    LogDebug("-> start outside and end inside");
                    MapVertex *splitVertex = EditAddVertex(map, intersection.p1);
                    DoSplit(map, &sectorsToUpdate, mapLine, splitVertex);
                    line_t line1 = { line.a, mline.a };
                    if(!Enqueue(&queue, line1, true)) return false;
                    LogDebug("Add 1 %s(%s:%d)", __FUNCTION__, __FILE__, __LINE__);
                    mapLine = NULL;
                }
                else if(gt(u0, 1) && gt(u1, 0)) // start outside and end inside reverse
                {
                    LogDebug("-> start outside and end inside reverse");
                    MapVertex *splitVertex = EditAddVertex(map, intersection.p1);
                    DoSplit(map, &sectorsToUpdate, mapLine, splitVertex);
                    line_t line1 = { line.a, mline.b };
                    if(!Enqueue(&queue, line1, true)) return false;
                    LogDebug("Add 1 %s:%d", __FILE__, __LINE__);
                    mapLine = NULL;
                }
                else if(gt(u0, 0) && gt(u1, 1)) // start inside and end outside
                {
                    LogDebug("-> start inside and end outside");
                    MapVertex *splitVertex = EditAddVertex(map, intersection.p0);
                    DoSplit(map, &sectorsToUpdate, mapLine, splitVertex);
                    line_t line1 = { mline.b, line.b };
                    if(!Enqueue(&queue, line1, true)) return false;
                    LogDebug("Add 1 %s(%s:%d)", __FUNCTION__, __FILE__, __LINE__);
                    mapLine = NULL;
                }
                else if(lt(u0, 1) && lt(u1, 0)) // start inside and end outside reverse
                {
                    LogDebug("-> start inside and end outside reverse");
                    MapVertex *splitVertex = EditAddVertex(map, intersection.p0);
                    DoSplit(map, &sectorsToUpdate, mapLine, splitVertex);
                    line_t line1 = { mline.a, line.b };
                    if(!Enqueue(&queue, line1, true)) return false;
                    LogDebug("Add 1 %s(%s:%d)", __FUNCTION__, __FILE__, __LINE__);
                    mapLine = NULL;
                }
                else
                {
                    line_t line1;
                    line_t line2;
                    if(gt(u0, u1))
                    {
                        LogDebug("-> start and end outside reverse");
                        line1 = (line_t){ line.a, mline.b };
                        line2 = (line_t){ mline.a, line.b };
                    }
                    else
                    {
                        LogDebug("-> start and end outside");
                        line1 = (line_t){ line.a, mline.a };
                        line2 = (line_t){ mline.b, line.b };
                    }
                    if(!Enqueue(&queue, line1, true)) return false;
                    if(!Enqueue(&queue, line2, true)) return false;
                    LogDebug("Add 2 %s(%s:%d)", __FUNCTION__, __FILE__, __LINE__);
                    mapLine = NULL;
                }
            }
            else if(LineIntersection(mline, line, &intersection))
            {
                double u = intersection.u, v = intersection.v;
                LogDebug("Intersection: U: %.32f, V: %.32f", u, v);
                if(eq(u, 0.0) || eq(u, 1.0))
                {
                    if(eq(v, 0.0) || eq(v, 1.0))
                    {
                        LogDebug("-> on each end");
                        mapLine->mark = true;
                        if(mapLine->frontSector)
                            InsertSectorUpdate(&sectorsToUpdate, mapLine, mapLine->frontSector->data);
                        if(mapLine->backSector)
                            InsertSectorUpdate(&sectorsToUpdate, mapLine, mapLine->backSector->data);
                    }
                    else // if(v > 0 || v < 1)
                    {
                        LogDebug("-> on mapline end, split the new line");
                        line_t line1 = { .a = line.a, .b = intersection.p0 };
                        line_t line2 = { .a = intersection.p0, .b = line.b };
                        if(!eq(vec2_distance2(line1.a, line1.b), 0))
                            if(!Enqueue(&queue, line1, true)) return false;
                        if(!eq(vec2_distance2(line2.a, line2.b), 0))
                            if(!Enqueue(&queue, line2, true)) return false;
                        LogDebug("-> add line1 length: %f", vec2_distance(line1.b, line1.a));
                        LogDebug("-> add line2 length: %f", vec2_distance(line2.b, line2.a));
                        LogDebug("Add 2 %s(%s:%d)", __FUNCTION__, __FILE__, __LINE__);
                        mapLine = NULL;
                    }
                }
                else
                {
                    if(eq(v, 0.0) || eq(v, 1.0))
                    {
                        LogDebug("-> intersection on one end of the new line");
                        MapVertex *closestVert = FindClosestVertex(map, intersection.p0, STITCHING_DIST);
                        if(!closestVert)
                        {
                            MapVertex *splitVertex = EditAddVertex(map, intersection.p0);
                            DoSplit(map, &sectorsToUpdate, mapLine, splitVertex);
                            if(!Enqueue(&queue, line, true)) return false;
                            LogDebug("Add 1 %s(%s:%d)", __FUNCTION__, __FILE__, __LINE__);
                        }
                        else
                        {
                            if(eq(v, 0.0))
                                line.b = closestVert->pos;
                            else
                                line.a = closestVert->pos;
                            if(!Enqueue(&queue, line, true)) return false;
                            LogDebug("Add 1 %s(%s:%d)", __FUNCTION__, __FILE__, __LINE__);
                        }
                        mapLine = NULL;
                    }
                    else
                    {
                        LogDebug("-> normal intersection");
                        line_t line1 = { .a = line.a, .b = intersection.p0 };
                        line_t line2 = { .a = intersection.p0, .b = line.b };
                        MapVertex *closestVert = FindClosestVertex(map, intersection.p0, STITCHING_DIST);
                        if(closestVert)
                        {
                            LogDebug("-> found a closer vertex");
                            line1.b = closestVert->pos;
                            line2.a = closestVert->pos;
                        }
                        else
                        {
                            MapVertex *splitVertex = EditAddVertex(map, intersection.p0);
                            DoSplit(map, &sectorsToUpdate, mapLine, splitVertex);
                        }
                        LogDebug("-> add line1 length: %f", mag(vec2_sub(line1.b, line1.a)));
                        LogDebug("-> add line2 length: %f", mag(vec2_sub(line2.b, line2.a)));
                        if(!eq(vec2_distance2(line1.a, line1.b), 0))
                            if(!Enqueue(&queue, line1, true)) return false;
                        if(!eq(vec2_distance2(line2.a, line2.b), 0))
                            if(!Enqueue(&queue, line2, true)) return false;
                        LogDebug("Add 2 %s(%s:%d)", __FUNCTION__, __FILE__, __LINE__);
                        mapLine = NULL;
                    }
                }
            }
            else
            {
                LogDebug("No Intersection or Overlap");
            }

            if(mapLine)
                mapLine = mapLine->next;
            else
                canInsertLine = false;
        }

        if(canInsertLine)
        {
            MapVertex *mva = FindClosestVertex(map, line.a, STITCHING_DIST);
            if(!mva) mva = EditAddVertex(map, line.a);
            MapVertex *mvb = FindClosestVertex(map, line.b, STITCHING_DIST);
            if(!mvb) mvb = EditAddVertex(map, line.b);
            if(!mva || !mvb) return false;

            MapLine *newMapLine = EditAddLine(map, mva, mvb, DefaultLineData());
            if(!newMapLine) return false;

            numNewLines++;
            newMapLine->new = potentialStart;
        }

        didIntersect |= !canInsertLine;
    }
    LogDebug("Done inserting...");

    for(MapLine *line = map->headLine; line; line = line->next)
    {
        if(!line->mark) continue;
        for(size_t i = 0; i < sectorsToUpdate.count; ++i)
        {
            if(line == sectorsToUpdate.items[i].line)
            {
                SectorData data = sectorsToUpdate.items[i].sectorData;
                MakeMapSector(map, line, data);
            }
        }
        line->mark = false;
    }

    arena_reset(&sectorUpdateArena);

    // create sectors from the new lines
    if(isLoop)
    {
        if(numNewLines == 0) // no lines were added, possibly filling an empty space surrounded by existing lines
        {
            MapLine *l = GetMapLine(map, (line_t){ .a = vertices[0], .b = vertices[1] });
            if(l && !(l->frontSector != NULL && l->backSector != NULL))
                MakeMapSector(map, l, DefaultSectorData());
        }
        else
        {
            for(MapLine *line = map->headLine; line; line = line->next)
            {
                if(!line->new) continue;
                if(line->frontSector == NULL)
                    MakeMapSector(map, line, DefaultSectorData());
                line->new = false;
            }
        }
    }

    return true;
}
