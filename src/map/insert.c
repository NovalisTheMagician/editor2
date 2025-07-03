#include "insert.h"

#include <assert.h>
#include <stdlib.h>

#include "arena.h"

#include "../edit.h"
#include "../geometry.h"
#include "../map.h"
#include "remove.h"
#include "triangulate.h"
#include "util.h"
#include "query.h"

#define MAX_LINES_PER_SECTOR 1024

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
    size_t numLines = FindOuterLineLoop(map, startLine, sectorLines, MAX_LINES_PER_SECTOR);
    if(FindEquvivalentSector(map, numLines, sectorLines)) return NULL;
    struct Polygon *poly = PolygonFromMapLines(map, numLines, sectorLines);

    Arena arena = { 0 };

    size_t numInnerLineLoops = 0, sizeInnerLineLoops = MAX_LINES_PER_SECTOR, usedLinesTop = 0, usedLinesSize = 4096, numPotentialLines = 0, sizePotentialLines = 1024;
    MapLine ***innerLines = arena_alloc(&arena, sizeInnerLineLoops * sizeof *innerLines);
    size_t *innerLinesNum = arena_alloc(&arena, sizeInnerLineLoops * sizeof *innerLinesNum);
    MapLine **usedLines = arena_alloc(&arena, usedLinesSize * sizeof *usedLines);
    MapLine **potentialLines = arena_alloc(&arena, sizePotentialLines * sizeof *potentialLines);

    for(size_t i = 0; i < map->lineList.count; ++i)
    {
        MapLine *line = &map->lineList.items[i];
        if(includes(numLines, (void**)sectorLines, line))
            continue;

        MapVertex *a = GetVertex(map, line->a), *b = GetVertex(map, line->b);

        bool aIn = PointInPolygon(poly, a->pos);
        bool bIn = PointInPolygon(poly, b->pos);
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
            size_t n = FindInnerLineLoop(map, potentialLine, innerLines[id], MAX_LINES_PER_SECTOR);
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
    bool hasFrontSector = line->frontSector != -1;
    bool hasBackSector = line->backSector != -1;
    bool hasSectorsAttached = hasFrontSector || hasBackSector;

    if(hasFrontSector)
    {
        MapSector *sector = GetSector(map, line->frontSector);
        frontData = CopySectorData(sector->data);
        for(size_t i = 0; i < sector->numOuterLines; ++i)
        {
            MapLine *sline = GetLine(map, sector->outerLines[i]);
            if(sline == line) continue;
            sline->mark = true;
            InsertSectorUpdate(sectorUpdate, sline, frontData);
        }
        RemoveSector(map, sector);
    }
    if(hasBackSector)
    {
        MapSector *sector = GetSector(map, line->backSector);
        backData = CopySectorData(sector->data);
        for(size_t i = 0; i < sector->numOuterLines; ++i)
        {
            MapLine *sline = GetLine(map, sector->outerLines[i]);
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
    bool hasFrontSector = line->frontSector != -1;
    bool hasBackSector = line->backSector != -1;
    bool hasSectorsAttached = hasFrontSector || hasBackSector;

    if(hasFrontSector)
    {
        MapSector *sector = GetSector(map, line->frontSector);
        frontData = CopySectorData(sector->data);
        for(size_t i = 0; i < sector->numOuterLines; ++i)
        {
            MapLine *sline = GetLine(map, sector->outerLines[i]);
            if(sline == line) continue;
            sline->mark = true;
            InsertSectorUpdate(sectorUpdate, sline, frontData);
        }
        RemoveSector(map, sector);
    }
    if(hasBackSector)
    {
        MapSector *sector = GetSector(map, line->backSector);
        backData = CopySectorData(sector->data);
        for(size_t i = 0; i < sector->numOuterLines; ++i)
        {
            MapLine *sline = GetLine(map, sector->outerLines[i]);
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

bool InsertLinesIntoMap(Map *map, size_t numVerts, vec2s vertices[static numVerts], bool isLoop)
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
                vec2s tmp = vertices[i];
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
        vec2s a = vertices[i];
        vec2s b = vertices[(i+1) % numVerts];

        if(!Enqueue(&queue, (line_t){ .a = a, .b = b }, i == 0)) return false;
    }

    while(queue.numLines > 0)
    {
        QueueElement el = Dequeue(&queue);
        line_t line = el.line;
        bool potentialStart = el.potentialStart;

        bool canInsertLine = true;
        for(size_t i = 0; i < map->lineList.count; ++i)
        {
            MapLine *mapLine = &map->lineList.items[i];
            MapVertex *a = GetVertex(map, mapLine->a), *b = GetVertex(map, mapLine->b);
            line_t mline = { .a = a->pos, .b = b->pos };

            classify_res_t result = ClassifyLines(mline, line);
            switch(result.type)
            {
            case NO_RELATION:
                {
                    
                }
                break;
            case SAME_LINES:
                {
                    canInsertLine = false;
                    goto doneLineLoop;
                }
                break;
            case INTERSECTION:
                {
                    MapVertex *splitVertex = FindClosestVertex(map, result.intersection.splitPoint, 1.0f);
                    if(!splitVertex) splitVertex = EditAddVertex(map, result.intersection.splitPoint);
                    DoSplit(map, &sectorsToUpdate, mapLine, splitVertex);
                    if(!Enqueue(&queue, result.intersection.splitLine1, true)) return false;
                    if(!Enqueue(&queue, result.intersection.splitLine2, true)) return false;
                    canInsertLine = false;
                    goto doneLineLoop;
                }
                break;
            case TOUCH:
                {
                    MapVertex *splitVertex = FindClosestVertex(map, result.touch.splitPoint, 2.0f);
                    if(!splitVertex) splitVertex = EditAddVertex(map, result.touch.splitPoint);
                    DoSplit(map, &sectorsToUpdate, mapLine, splitVertex);
                    if(!Enqueue(&queue, line, true)) return false;
                    canInsertLine = false;
                    goto doneLineLoop;
                }
                break;
            case TOUCH_REVERSE:
                {
                    if(!Enqueue(&queue, (line_t){ .a = line.a, .b = result.touch.splitPoint }, true)) return false;
                    if(!Enqueue(&queue, (line_t){ .a = result.touch.splitPoint, .b = line.b }, true)) return false;
                    canInsertLine = false;
                    goto doneLineLoop;
                }
                break;
            case OVERLAP:
                {
                    MapVertex *splitVertex = FindClosestVertex(map, result.overlap.splitPoint, 2.0f);
                    if(!splitVertex) splitVertex = EditAddVertex(map, result.overlap.splitPoint);
                    DoSplit(map, &sectorsToUpdate, mapLine, splitVertex);
                    if(!Enqueue(&queue, result.overlap.line, true)) return false;
                    canInsertLine = false;
                    goto doneLineLoop;
                }
                break;
            case SIMPLE_OVERLAP_INNER:
                {
                    MapVertex *splitVertex = FindClosestVertex(map, result.overlap.splitPoint, 2.0f);
                    if(!splitVertex) splitVertex = EditAddVertex(map, result.overlap.splitPoint);
                    DoSplit(map, &sectorsToUpdate, mapLine, splitVertex);
                    canInsertLine = false;
                    goto doneLineLoop;
                }
                break;
            case SIMPLE_OVERLAP_OUTER:
                {
                    if(!Enqueue(&queue, result.overlap.line, false)) return false;
                    canInsertLine = false;
                    goto doneLineLoop;
                }
                break;
            case INNER_CONTAINMENT:
                {
                    MapVertex *splitVertexA = FindClosestVertex(map, result.innerContainment.split1, 2.0f);
                    if(!splitVertexA) splitVertexA = EditAddVertex(map, result.innerContainment.split1);
                    MapVertex *splitVertexB = FindClosestVertex(map, result.innerContainment.split2, 2.0f);
                    if(!splitVertexB) splitVertexB = EditAddVertex(map, result.innerContainment.split2);
                    DoSplit2(map, &sectorsToUpdate, mapLine, splitVertexA, splitVertexB);
                    canInsertLine = false;
                    goto doneLineLoop;
                }
                break;
            case OUTER_CONTAINMENT:
                {
                    if(!Enqueue(&queue, result.outerContainment.line1, true)) return false;
                    if(!Enqueue(&queue, result.outerContainment.line2, true)) return false;
                    canInsertLine = false;
                    goto doneLineLoop;
                }
                break;
            }
        }
doneLineLoop:

        if(canInsertLine)
        {
            MapVertex *mva = EditAddVertex(map, line.a);
            MapVertex *mvb = EditAddVertex(map, line.b);
            if(!mva || !mvb) return false;

            MapLine *newMapLine = EditAddLine(map, mva, mvb, DefaultLineData());
            if(!newMapLine) return false;

            numNewLines++;
            newMapLine->new = potentialStart;
        }

        didIntersect |= !canInsertLine;
    }

    for(size_t i = 0; i < map->lineList.count; ++i)
    {
        MapLine *line = &map->lineList.items[i];
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
            if(l && !(l->frontSector != -1 && l->backSector != -1))
                MakeMapSector(map, l, DefaultSectorData());
        }
        else
        {
            for(size_t i = 0; i < map->lineList.count; ++i)
            {
                MapLine *line = &map->lineList.items[i];
                if(!line->new) continue;
                if(line->frontSector == -1)
                    MakeMapSector(map, line, DefaultSectorData());
                line->new = false;
            }
        }
    }

    return true;
}
