#include "insert.h"

#include <assert.h>

#include "../edit.h"
#include "../geometry.h"
#include "map.h"
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
        bool valid;
    } data[QUEUE_SIZE];
    size_t length;
} SectorUpdate;

static inline void InsertSectorUpdate(SectorUpdate *sectorUpdate, MapLine *line, SectorData sectorData, bool front)
{
    assert(sectorUpdate->length <= QUEUE_SIZE);
    sectorUpdate->data[sectorUpdate->length++] = (typeof(sectorUpdate->data[0])){ .line = line, .sectorData = sectorData, .valid = true, .front = front };
}

static void RemoveSectorUpdate(SectorUpdate *sectorUpdate, MapLine *line)
{
    for(size_t i = 0; i < sectorUpdate->length; ++i)
    {
        if(sectorUpdate->data[i].line == line)
        {
            sectorUpdate->data[i].valid = false;
            sectorUpdate->data[i].line = NULL;
            FreeSectorData(sectorUpdate->data[i].sectorData);
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

    SectorUpdate sectorsToUpdate = {0};

    // insert the drawn lines into queue
    for(size_t i = 0; i < end; ++i)
    {
        vec2s a = vertices[i];
        vec2s b = vertices[(i+1) % numVerts];

        Enqueue(&queue, (line_t){ .a = a, .b = b }, i == 0);

        assert(queue.tail != queue.head);
    }

    while(queue.numLines > 0)
    {
        QueueElement el = Dequeue(&queue);
        line_t line = el.line;
        bool potentialStart = el.potentialStart;

        bool canInsertLine = true;
        bool intersect;
        MapLine *mapLine = map->headLine;
        while(mapLine != NULL)
        {
            line_t mline = { .a = mapLine->a->pos, .b = mapLine->b->pos };
            if(LineEq(mline, line)) //line already exists, discard it
            {
                canInsertLine = false;
                break;
            }

            if(glms_vec2_eqv_eps(mline.a, line.a) || glms_vec2_eqv_eps(mline.b, line.a) || glms_vec2_eqv_eps(mline.b, line.b) || glms_vec2_eqv_eps(mline.a, line.b)) // line shares one of the points with the mapline
            {
                if(LineIsParallel(mline, line)) // overlap
                {
                    vec2s commonPoint = LineGetCommonPoint(mline, line);
                    line_t major = { .a = commonPoint, .b = glms_vec2_eqv_eps(commonPoint, mline.a) ? mline.b : mline.a };
                    line_t minor = { .a = commonPoint, .b = glms_vec2_eqv_eps(commonPoint, line.a) ? line.b : line.a };

                    vec2s u = glms_vec2_sub(major.b, major.a);
                    vec2s v = glms_vec2_sub(minor.b, minor.a);
                    float dot = glms_vec2_dot(u, v);
                    if(dot > 0) // they do overlap
                    {
                        float t = LineGetPointFactor(major, minor.b);
                        if(t > 1)
                        {
                            Enqueue(&queue, (line_t){ .a = major.b, .b = minor.b}, false);

                            assert(queue.numLines < QUEUE_SIZE);
                        }
                        else
                        {
                            MapVertex *splitVertex = EditAddVertex(map, minor.b);
                            MapLine *lineToSplit = mapLine;
                            DoSplit(map, &sectorsToUpdate, lineToSplit, splitVertex);
                            mapLine = NULL; // since we are splitting the line here we should stop iterating through the rest of the map lines
                        }
                        canInsertLine = false;
                        break;
                    }
                }
                else // cant overlap or intersect line
                {
                    mapLine = mapLine->next;
                    continue;
                }
            }

            float mlOrient = (mline.b.x - mline.a.x) * (mline.b.y + mline.a.y);
            float lOrient = (line.b.x - line.a.x) * (line.b.y + line.a.y);

            line_t lineCorrected = line;
            if(signbit(mlOrient) != signbit(lOrient))
            {
                lineCorrected.a = line.b;
                lineCorrected.b = line.a;
            }

            intersection_res_t result = {0};
            if((intersect = LineIntersection(mline, line, &result)))
            {
                if(!glms_vec2_eqv_eps(mline.a, result.p0) && !glms_vec2_eqv_eps(mline.b, result.p0))
                {
                    MapVertex *splitVertex = EditAddVertex(map, result.p0);
                    MapLine *lineToSplit = mapLine;
                    DoSplit(map, &sectorsToUpdate, lineToSplit, splitVertex);
                }

                if(!glms_vec2_eqv_eps(line.a, result.p0))
                {
                    Enqueue(&queue, (line_t){ line.a, result.p0 }, true);
                }

                if(!glms_vec2_eqv_eps(result.p0, line.b))
                {
                    Enqueue(&queue, (line_t){ result.p0, line.b }, true);
                }

                mapLine = NULL;

                assert(queue.numLines < QUEUE_SIZE);
            }
            else if((intersect = LineOverlap(mline, lineCorrected, &result)))
            {
                if(result.t0 == 0 && result.t1 == 1)
                {
                    MapVertex *splitVertexA = EditAddVertex(map, result.p0);
                    MapVertex *splitVertexB = EditAddVertex(map, result.p1);
                    MapLine *lineToSplit = mapLine;
                    mapLine = NULL;
                    DoSplit2(map, &sectorsToUpdate, lineToSplit, splitVertexA, splitVertexB);
                }
                else if(result.t0 == 0)
                {
                    MapVertex *splitVertex = EditAddVertex(map, result.p0);
                    MapLine *lineToSplit = mapLine;
                    mapLine = NULL;
                    DoSplit(map, &sectorsToUpdate, lineToSplit, splitVertex);

                    Enqueue(&queue, (line_t){ mline.b, lineCorrected.b }, true);

                    assert(queue.numLines < QUEUE_SIZE);
                }
                else if(result.t1 == 0)
                {
                    MapVertex *splitVertex = EditAddVertex(map, result.p1);
                    MapLine *lineToSplit = mapLine;
                    mapLine = NULL;
                    DoSplit(map, &sectorsToUpdate, lineToSplit, splitVertex);

                    Enqueue(&queue, (line_t){ mline.a, lineCorrected.a }, true);

                    assert(queue.numLines < QUEUE_SIZE);
                }
                else
                {
                    Enqueue(&queue, (line_t){ mline.b, lineCorrected.b }, true);
                    Enqueue(&queue, (line_t){ lineCorrected.a, mline.a }, true);

                    mapLine = NULL;

                    assert(queue.numLines < QUEUE_SIZE);
                }
            }
            else
                mapLine = mapLine->next;
            canInsertLine &= !intersect;
        }

        if(canInsertLine)
        {
            MapVertex *mva = EditAddVertex(map, line.a);
            MapVertex *mvb = EditAddVertex(map, line.b);
            if(!mva || !mvb) return false;

            MapLine *line = EditAddLine(map, mva, mvb, DefaultLineData());
            if(!line) return false;

            if(potentialStart) startLines[numStartLines++] = line;
        }

        didIntersect |= !canInsertLine;
    }

    for(size_t i = 0; i < sectorsToUpdate.length; ++i)
    {
        if(!sectorsToUpdate.data[i].valid) continue;
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
