#include "insert.h"

#include <assert.h>

#include "../edit.h"
#include "../geometry.h"
#include "../map.h"
#include "remove.h"
#include "triangulate.h"
#include "util.h"
#include "query.h"

#define MAX_LINES_PER_SECTOR 1024

MapSector* MakeMapSector(Map *map, MapLine *startLine, bool front, SectorData data)
{
    // front means natural direction
    MapLine *sectorLines[MAX_LINES_PER_SECTOR] = { 0 };
    bool lineFront[MAX_LINES_PER_SECTOR] = { 0 };
    size_t numLines = FindOuterLineLoop(startLine, front, sectorLines, lineFront, MAX_LINES_PER_SECTOR);
    struct Polygon *poly = PolygonFromMapLines(numLines, sectorLines, lineFront);

    size_t numInnerLineLoops = 0, sizeInnerLineLoops = 8, usedLinesTop = 0, usedLinesSize = 1024, numPotentialLines = 0, sizePotentialLines = 1024;
    MapLine ***innerLines = calloc(sizeInnerLineLoops, sizeof *innerLines);
    bool **innerLinesFront = calloc(sizeInnerLineLoops, sizeof *innerLinesFront);
    size_t *innerLinesNum = calloc(sizeInnerLineLoops, sizeof *innerLinesNum);
    MapLine **usedLines = calloc(usedLinesSize, sizeof *usedLines);
    MapLine **potentialLines = calloc(sizePotentialLines, sizeof *potentialLines);

    for(MapLine *line = map->headLine; line; line = line->next)
    {
        bool aIn = PointInPolygon(poly, line->a->pos);
        bool bIn = PointInPolygon(poly, line->b->pos);
        if(aIn && bIn)
            potentialLines[numPotentialLines++] = line;
    }

    while(numPotentialLines > 0)
    {
        size_t id = numInnerLineLoops++;
        innerLines[id] = calloc(MAX_LINES_PER_SECTOR, sizeof **innerLines);
        innerLinesFront[id] = calloc(MAX_LINES_PER_SECTOR, sizeof **innerLinesFront);
        size_t n = FindInnerLineLoop(potentialLines[0], true, innerLines[numInnerLineLoops-1], innerLinesFront[numInnerLineLoops-1], MAX_LINES_PER_SECTOR);
        if(n == 0) // couldnt find a loop
        {

        }
    }

    free(poly);
    MapSector *sector = EditAddSector(map, numLines, sectorLines, lineFront, numInnerLineLoops, innerLinesNum, innerLines, innerLinesFront, data);
    free(usedLines);
    free(potentialLines);
    for(size_t i = 0; i < numInnerLineLoops; ++i)
    {
        free(innerLines[i]);
        free(innerLinesFront[i]);
    }
    free(innerLinesNum);
    free(innerLines);
    free(innerLinesFront);
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

typedef struct SectorUpdate
{
    struct
    {
        MapLine *line;
        SectorData sectorData;
        bool front;
    } data[QUEUE_SIZE];
    size_t length;
} SectorUpdate;

static inline void InsertSectorUpdate(SectorUpdate *sectorUpdate, MapLine *line, SectorData sectorData, bool front)
{
    assert(sectorUpdate->length <= QUEUE_SIZE);
    sectorUpdate->data[sectorUpdate->length++] = (typeof(sectorUpdate->data[0])){ .line = line, .sectorData = sectorData, .front = front };
}

static void RemoveSectorUpdate(SectorUpdate *sectorUpdate, MapLine *line)
{
    for(size_t i = 0; i < sectorUpdate->length; ++i)
    {
        if(sectorUpdate->data[i].line == line)
        {
            FreeSectorData(sectorUpdate->data[i].sectorData);
            sectorUpdate->data[i] = sectorUpdate->data[sectorUpdate->length-1];
            sectorUpdate->length--;
            break;
        }
    }
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
        RemoveSector(map, line->frontSector);
    }
    if(hasBackSector)
    {
        backData = CopySectorData(line->backSector->data);
        RemoveSector(map, line->backSector);
    }

    if(hasSectorsAttached) RemoveSectorUpdate(sectorUpdate, line);
    SplitResult result = SplitMapLine(map, line, vertex);
    if(hasSectorsAttached)
    {
        if(hasFrontSector)
        {
            InsertSectorUpdate(sectorUpdate, result.left, frontData, true);
            InsertSectorUpdate(sectorUpdate, result.right, frontData, true);
        }
        if(hasBackSector)
        {
            InsertSectorUpdate(sectorUpdate, result.left, backData, false);
            InsertSectorUpdate(sectorUpdate, result.right, backData, false);
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
        RemoveSector(map, line->frontSector);
    }
    if(hasBackSector)
    {
        backData = CopySectorData(line->backSector->data);
        RemoveSector(map, line->backSector);
    }

    if(hasSectorsAttached) RemoveSectorUpdate(sectorUpdate, line);
    SplitResult result = SplitMapLine2(map, line, vertexA, vertexB);
    if(hasSectorsAttached)
    {
        if(hasFrontSector)
        {
            InsertSectorUpdate(sectorUpdate, result.left, frontData, true);
            InsertSectorUpdate(sectorUpdate, result.right, frontData, true);
            InsertSectorUpdate(sectorUpdate, result.middle, frontData, true);
        }
        if(hasBackSector)
        {
            InsertSectorUpdate(sectorUpdate, result.left, backData, false);
            InsertSectorUpdate(sectorUpdate, result.right, backData, false);
            InsertSectorUpdate(sectorUpdate, result.middle, backData, false);
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

    MapLine *startLines[QUEUE_SIZE];
    size_t numStartLines = 0;

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
        MapLine *mapLine = map->headLine;
        while(mapLine != NULL)
        {
            line_t mline = { .a = mapLine->a->pos, .b = mapLine->b->pos };

            classify_res_t result = ClassifyLines(mline, line);
            switch(result.type)
            {
            case NO_RELATION:
                break;
            case SAME_LINES:
                {
                    mapLine = NULL;
                }
                break;
            case INTERSECTION:
                {
                    MapVertex *splitVertex = EditAddVertex(map, result.intersection.splitPoint);
                    DoSplit(map, &sectorsToUpdate, mapLine, splitVertex);
                    if(!Enqueue(&queue, result.intersection.splitLine1, true)) return false;
                    if(!Enqueue(&queue, result.intersection.splitLine2, true)) return false;
                    mapLine = NULL;
                }
                break;
            case TOUCH:
                {
                    MapVertex *splitVertex = EditAddVertex(map, result.touch.splitPoint);
                    DoSplit(map, &sectorsToUpdate, mapLine, splitVertex);
                    if(!Enqueue(&queue, line, true)) return false;
                    mapLine = NULL;
                }
                break;
            case TOUCH_REVERSE:
                {
                    if(!Enqueue(&queue, (line_t){ .a = line.a, .b = result.touch.splitPoint }, true)) return false;
                    if(!Enqueue(&queue, (line_t){ .a = result.touch.splitPoint, .b = line.b }, true)) return false;
                    mapLine = NULL;
                }
                break;
            case OVERLAP:
                {
                    MapVertex *splitVertex = EditAddVertex(map, result.overlap.splitPoint);
                    DoSplit(map, &sectorsToUpdate, mapLine, splitVertex);
                    if(!Enqueue(&queue, result.overlap.line, true)) return false;
                    mapLine = NULL;
                }
                break;
            case SIMPLE_OVERLAP_INNER:
                {
                    MapVertex *splitVertex = EditAddVertex(map, result.overlap.splitPoint);
                    DoSplit(map, &sectorsToUpdate, mapLine, splitVertex);
                    mapLine = NULL;
                }
                break;
            case SIMPLE_OVERLAP_OUTER:
                {
                    if(!Enqueue(&queue, result.overlap.line, false)) return false;
                    mapLine = NULL;
                }
                break;
            case INNER_CONTAINMENT:
                {
                    MapVertex *splitVertexA = EditAddVertex(map, result.innerContainment.split1);
                    MapVertex *splitVertexB = EditAddVertex(map, result.innerContainment.split2);
                    DoSplit2(map, &sectorsToUpdate, mapLine, splitVertexA, splitVertexB);
                    mapLine = NULL;
                }
                break;
            case OUTER_CONTAINMENT:
                {
                    if(!Enqueue(&queue, result.outerContainment.line1, true)) return false;
                    if(!Enqueue(&queue, result.outerContainment.line2, true)) return false;
                    mapLine = NULL;
                }
                break;
            }

            if(mapLine)
                mapLine = mapLine->next;
            else
                canInsertLine = false;
        }

        if(canInsertLine)
        {
            MapVertex *mva = EditAddVertex(map, line.a);
            MapVertex *mvb = EditAddVertex(map, line.b);
            if(!mva || !mvb) return false;

            MapLine *newMapLine = EditAddLine(map, mva, mvb, DefaultLineData());
            if(!newMapLine) return false;

            if(potentialStart) startLines[numStartLines++] = newMapLine;
        }

        didIntersect |= !canInsertLine;
    }

    for(size_t i = 0; i < sectorsToUpdate.length; ++i)
    {
        bool front = sectorsToUpdate.data[i].front;
        MapLine *line = sectorsToUpdate.data[i].line;
        SectorData data = sectorsToUpdate.data[i].sectorData;
        if(front && line->frontSector != NULL) continue;
        if(!front && line->backSector != NULL) continue;
        MakeMapSector(map, line, front, data);
        FreeSectorData(data);
    }

    // create sectors from the new lines
    if(isLoop)
    {
        if(numStartLines == 0) // no lines were added, possibly filling an empty space surrounded by existing lines
        {
            MapLine *l = GetMapLine(map, (line_t){ .a = vertices[0], .b = vertices[1] });
            assert(l);
            if(!(l->frontSector != NULL && l->backSector != NULL))
                MakeMapSector(map, l, l->frontSector == NULL, DefaultSectorData());
        }
        else
        {
            for(size_t i = 0; i < numStartLines; ++i)
            {
                if(startLines[i]->frontSector == NULL)
                    MakeMapSector(map, startLines[i], true, DefaultSectorData());
            }
        }
    }

    return true;
}
