#include "insert.h"

#include <assert.h>

#include "../edit.h"
#include "../geometry.h"
#include "map.h"
#include "map/create.h"
#include "remove.h"
#include "util.h"

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

static int angleSort(const void *a, const void *b)
{
    const Path *aPath = a;
    const Path *bPath = b;
    if(aPath->relativeAngle > bPath->relativeAngle) return -1;
    if(aPath->relativeAngle < bPath->relativeAngle) return 1;
    return 0;
}

static void insertPath(PathStack *pathStack, MapLine *line, MapVertex *nextVertex, MapVertex *vertex)
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
    qsort(pathStack->paths, pathStack->numPaths, sizeof *pathStack->paths, angleSort);
}

MapSector* MakeMapSector(Map *map, MapLine *startLine, bool front, SectorData data)
{
    // front means natural direction
    MapLine *sectorLines[1024] = { startLine };
    bool lineFront[1024] = { front };
    size_t numLines = 1;

    MapVertex *mapVertexForNext = front ? startLine->b : startLine->a, *mapVertex = front ? startLine->a : startLine->b;
    PathStack *pathStack = calloc(1024, sizeof *pathStack);
    size_t pathTop = 0;
    insertPath(&pathStack[pathTop++], startLine, mapVertexForNext, mapVertex);

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
        MapVertex *mapVertex = pathElement->vertex;

        // found a loop
        if(mapLine == startLine)
            break;

        // add current line to list
        size_t idx = numLines++;
        sectorLines[idx] = mapLine;
        lineFront[idx] = IsLineFront(mapVertex, mapLine);

        PathStack *stackPush = &pathStack[pathTop++];
        insertPath(stackPush, mapLine, path.nextVertex, pathElement->vertex);
    }

    free(pathStack);

    LogDebug("Found a loop with %d lines", numLines);
    return EditAddSector(map, numLines, sectorLines, lineFront, data);
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

static inline void Enqueue(LineQueue *queue, line_t line, bool potentialStart)
{
    queue->elements[queue->tail] = (typeof(queue->elements[queue->tail])){ .line = line, .potentialStart = potentialStart };
    queue->tail = (queue->tail + 1) % QUEUE_SIZE;
    queue->numLines++;
    assert(queue->numLines < QUEUE_SIZE);
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

        Enqueue(&queue, (line_t){ .a = a, .b = b }, i == 0);
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
                    if(result.intersection.hasSplit)
                    {
                        MapVertex *splitVertex = EditAddVertex(map, result.intersection.splitPoint);
                        DoSplit(map, &sectorsToUpdate, mapLine, splitVertex);
                    }
                    if(result.intersection.hasLine1) Enqueue(&queue, result.intersection.splitLine1, true);
                    if(result.intersection.hasLine2) Enqueue(&queue, result.intersection.splitLine2, true);
                    mapLine = NULL;
                }
                break;
            case TOUCH:
                {
                    MapVertex *splitVertex = EditAddVertex(map, result.touch.splitPoint);
                    DoSplit(map, &sectorsToUpdate, mapLine, splitVertex);
                    Enqueue(&queue, line, true);
                    mapLine = NULL;
                }
                break;
            case TOUCH_REVERSE:
                {
                    Enqueue(&queue, (line_t){ .a = line.a, .b = result.touch.splitPoint }, true);
                    Enqueue(&queue, (line_t){ .a = result.touch.splitPoint, .b = line.b }, true);
                    mapLine = NULL;
                }
                break;
            case OVERLAP:
                {
                    MapVertex *splitVertex = EditAddVertex(map, result.overlap.splitPoint);
                    DoSplit(map, &sectorsToUpdate, mapLine, splitVertex);
                    Enqueue(&queue, result.overlap.line, true);
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
                    Enqueue(&queue, result.overlap.line, false);
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
                    Enqueue(&queue, result.outerContainment.line1, true);
                    Enqueue(&queue, result.outerContainment.line2, true);
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
