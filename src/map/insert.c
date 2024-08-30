#include "insert.h"

#include <assert.h>

#include "../edit.h"
#include "../geometry.h"
#include "remove.h"
#include "util.h"

MapSector* MakeMapSector(EdState *state, MapLine *startLine, bool front, SectorData data)
{
    // front means natural direction
    MapLine *sectorLines[1024] = {0};
    bool lineFront[1024] = {0};
    size_t numLines = 0;

    MapLine *mapLine = startLine;
    MapVertex *mapVertexForNext = front ? mapLine->b : mapLine->a, *mapVertex = front ? mapLine->a : mapLine->b;
    while(true)
    {
        LogDebug("Iteration");
        // add current line to list
        sectorLines[numLines] = mapLine;
        lineFront[numLines++] = IsLineFront(mapVertex, mapLine);
        LogDebug("Is line front %d", lineFront[numLines-1]);

        // get the next line with the smallest angle
        MapLine *nextMapLine = NULL;
        float smallestAngle = FLT_MAX;
        for(size_t i = 0; i < mapVertexForNext->numAttachedLines; ++i)
        {
            MapLine *attLine = mapVertexForNext->attachedLines[i];
            if(attLine == mapLine) continue;

            MapVertex *otherVertex = mapVertexForNext == attLine->a ? attLine->b : attLine->a;
            float angle = PI2 - AngleOfLines((line_t){ .a = mapVertexForNext->pos, .b = mapVertex->pos }, (line_t){ .a = mapVertexForNext->pos, .b = otherVertex->pos });
            if(angle < smallestAngle)
            {
                smallestAngle = angle;
                nextMapLine = attLine;
            }
            LogDebug("Line: %d, Angle: %f", attLine->idx, rad2deg(angle));
        }

        // line ends here
        if(nextMapLine == NULL)
        {
            LogDebug("Line ends, cant create sector!");
            return NULL;
        }

        // we found a loop
        if(nextMapLine == startLine)
        {
            break;
        }

        mapLine = nextMapLine;
        mapVertex = mapVertexForNext;
        mapVertexForNext = mapLine->a == mapVertex ? mapLine->b : mapLine->a;
    }

    LogDebug("Found a loop with %d lines", numLines);
    return EditAddSector(state, numLines, sectorLines, lineFront, data);
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
    queue->elements[queue->tail] = (__typeof__(queue->elements[queue->tail])){ .line = line, .potentialStart = potentialStart };
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
    sectorUpdate->data[sectorUpdate->length++] = (__typeof__(sectorUpdate->data[0])){ .line = line, .sectorData = sectorData, .valid = true, .front = front };
}

static void RemoveSectorUpdate(SectorUpdate *sectorUpdate, MapLine *line)
{
    for(size_t i = 0; i < sectorUpdate->length; ++i)
    {
        if(sectorUpdate->data[i].line == line)
        {
            sectorUpdate->data[i].valid = false;
            break;
        }
    }
}

static void DoSplit(EdState *state, SectorUpdate *sectorUpdate, MapLine *line, MapVertex *vertex)
{
    Map *map = &state->map;

    SectorData frontData = DefaultSectorData();
    SectorData backData = DefaultSectorData();
    bool hasFrontSector = line->frontSector != NULL;
    bool hasBackSector = line->backSector != NULL;
    bool hasSectorsAttached = hasFrontSector || hasBackSector;

    if(hasFrontSector)
    {
        frontData = line->frontSector->data;
        RemoveSector(map, line->frontSector);
    }
    if(hasBackSector)
    {
        backData = line->backSector->data;
        RemoveSector(map, line->backSector);
    }

    if(hasSectorsAttached) RemoveSectorUpdate(sectorUpdate, line);
    SplitResult result = SplitMapLine(state, line, vertex);
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

static void DoSplit2(EdState *state, SectorUpdate *sectorUpdate, MapLine *line, MapVertex *vertexA, MapVertex *vertexB)
{
    Map *map = &state->map;

    SectorData frontData = DefaultSectorData();
    SectorData backData = DefaultSectorData();
    bool hasFrontSector = line->frontSector != NULL;
    bool hasBackSector = line->backSector != NULL;
    bool hasSectorsAttached = hasFrontSector || hasBackSector;

    if(hasFrontSector)
    {
        frontData = line->frontSector->data;
        RemoveSector(map, line->frontSector);
    }
    if(hasBackSector)
    {
        backData = line->backSector->data;
        RemoveSector(map, line->backSector);
    }

    if(hasSectorsAttached) RemoveSectorUpdate(sectorUpdate, line);
    SplitResult result = SplitMapLine2(state, line, vertexA, vertexB);
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

bool InsertLinesIntoMap(EdState *state, size_t numVerts, vec2s vertices[static numVerts], bool isLoop)
{
    Map *map = &state->map;
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
                    if(dot > 0) // they do infact overlap
                    {
                        float t = LineGetPointFactor(major, minor.b);
                        if(t > 1)
                        {
                            Enqueue(&queue, (line_t){ .a = major.b, .b = minor.b}, false);

                            assert(queue.numLines < QUEUE_SIZE);
                        }
                        else
                        {
                            MapVertex *splitVertex = EditAddVertex(state, minor.b);
                            MapLine *lineToSplit = mapLine;
                            DoSplit(state, &sectorsToUpdate, lineToSplit, splitVertex);
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
                    MapVertex *splitVertex = EditAddVertex(state, result.p0);
                    MapLine *lineToSplit = mapLine;
                    DoSplit(state, &sectorsToUpdate, lineToSplit, splitVertex);
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
                    MapVertex *splitVertexA = EditAddVertex(state, result.p0);
                    MapVertex *splitVertexB = EditAddVertex(state, result.p1);
                    MapLine *lineToSplit = mapLine;
                    mapLine = NULL;
                    DoSplit2(state, &sectorsToUpdate, lineToSplit, splitVertexA, splitVertexB);
                }
                else if(result.t0 == 0)
                {
                    MapVertex *splitVertex = EditAddVertex(state, result.p0);
                    MapLine *lineToSplit = mapLine;
                    mapLine = NULL;
                    DoSplit(state, &sectorsToUpdate, lineToSplit, splitVertex);

                    Enqueue(&queue, (line_t){ mline.b, lineCorrected.b }, true);

                    assert(queue.numLines < QUEUE_SIZE);
                }
                else if(result.t1 == 0)
                {
                    MapVertex *splitVertex = EditAddVertex(state, result.p1);
                    MapLine *lineToSplit = mapLine;
                    mapLine = NULL;
                    DoSplit(state, &sectorsToUpdate, lineToSplit, splitVertex);

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
            MapVertex *mva = EditAddVertex(state, line.a);
            MapVertex *mvb = EditAddVertex(state, line.b);
            if(!mva || !mvb) return false;

            MapLine *line = EditAddLine(state, mva, mvb, DefaultLineData());
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
        MakeMapSector(state, line, front, data);
    }

    // create sectors from the new lines
    if(isLoop)
    {
        if(numStartLines == 0) // no lines were added, possibly filling an empty space surrounded by existing lines
        {
            MapLine *l = GetMapLine(map, (line_t){ .a = vertices[0], .b = vertices[1] });
            assert(l);
            if(!(l->frontSector != NULL && l->backSector != NULL))
                MakeMapSector(state, l, l->frontSector == NULL, DefaultSectorData());
        }
        else
        {
            for(size_t i = 0; i < numStartLines; ++i)
            {
                if(startLines[i]->frontSector == NULL)
                    MakeMapSector(state, startLines[i], true, DefaultSectorData());
            }
        }
    }

    return true;
}
