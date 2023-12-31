#include "edit.h"
#include "map.h"

#include "geometry.h"

#include <assert.h>
#include <tgmath.h>

#include <triangulate.h>

static void RemoveVertex(struct Map map[static 1], struct MapVertex vertex[static 1])
{
    struct MapVertex *prev = vertex->prev;
    struct MapVertex *next = vertex->next;

    if(prev == NULL && next == NULL)
    {
        map->headVertex = map->tailVertex = NULL;
    }
    else if(vertex == map->headVertex)
    {
        next->prev = NULL;
        map->headVertex = next;
    }
    else if(vertex == map->tailVertex)
    {
        prev->next = NULL;
        map->tailVertex = prev;
    }
    else
    {
        prev->next = next;
        next->prev = prev;
    }

    for(size_t i = 0; i < vertex->numAttachedLines; ++i)
    {
        struct MapLine *line = vertex->attachedLines[i];
        if(line->a == vertex)
        {
            line->a = NULL;
        }
        else
        {
            line->b = NULL;
        }
    }

    FreeMapVertex(vertex);

    map->numVertices--;
    map->dirty = true;
}

/*
static void RemoveLine(struct Map *map, struct MapLine *line)
{
    assert(line);

    struct MapLine *prev = line->prev;
    struct MapLine *next = line->next;

    if(prev == NULL && next == NULL)
    {
        map->headLine = map->tailLine = NULL;
    }
    else if(line == map->headLine)
    {
        next->prev = NULL;
        map->headLine = next;
    }
    else if(line == map->tailLine)
    {
        prev->next = NULL;
        map->tailLine = prev;
    }
    else
    {
        prev->next = next;
        next->prev = prev;
    }

    if(line->a->numAttachedLines == 1)
    {
        RemoveVertex(map, line->a);
    }
    else
    {
        for(size_t i = line->aVertIndex + 1; i < line->a->numAttachedLines; ++i)
        {
            struct MapLine *attLine = line->a->attachedLines[i];
            attLine->a == line->a ? attLine->aVertIndex-- : attLine->bVertIndex--;
        }
        memmove(line->a->attachedLines + line->aVertIndex, line->a->attachedLines + line->aVertIndex + 1, ((line->a->numAttachedLines - (line->aVertIndex)) * sizeof *line->a->attachedLines));
        line->a->numAttachedLines--;
    }

    if(line->b->numAttachedLines == 1)
    {
        RemoveVertex(map, line->b);
    }
    else
    {
        for(size_t i = line->bVertIndex + 1; i < line->b->numAttachedLines; ++i)
        {
            struct MapLine *attLine = line->b->attachedLines[i];
            attLine->a == line->b ? attLine->aVertIndex-- : attLine->bVertIndex--;
        }
        memmove(line->b->attachedLines + line->bVertIndex, line->b->attachedLines + line->bVertIndex + 1, ((line->b->numAttachedLines - (line->bVertIndex)) * sizeof *line->b->attachedLines));
        line->b->numAttachedLines--;
    }

    pstr_free(line->front.lowerTex);
    pstr_free(line->front.middleTex);
    pstr_free(line->front.upperTex);
    pstr_free(line->back.lowerTex);
    pstr_free(line->back.middleTex);
    pstr_free(line->back.upperTex);
    free(line);

    map->numLines--;
    map->dirty = true;
}
*/

static void RemoveLine(struct Map map[static 1], struct MapLine line[static 1])
{
    struct MapLine *prev = line->prev;
    struct MapLine *next = line->next;

    if(prev == NULL && next == NULL)
    {
        map->headLine = map->tailLine = NULL;
    }
    else if(line == map->headLine)
    {
        next->prev = NULL;
        map->headLine = next;
    }
    else if(line == map->tailLine)
    {
        prev->next = NULL;
        map->tailLine = prev;
    }
    else
    {
        prev->next = next;
        next->prev = prev;
    }
    
    if(line->a && line->a->numAttachedLines > 0)
    {
        struct MapVertex *v = line->a;
        for(size_t i = line->aVertIndex + 1; i < v->numAttachedLines; ++i)
        {
            struct MapLine *attLine = v->attachedLines[i];
            attLine->a == v ? attLine->aVertIndex-- : attLine->bVertIndex--;
        }
        memmove(v->attachedLines + line->aVertIndex, v->attachedLines + line->aVertIndex + 1, (v->numAttachedLines - (line->aVertIndex)) * sizeof *v->attachedLines);
        v->numAttachedLines--;
        //memset(v->attachedLines + v->numAttachedLines, 0, COUNT_OF(v->attachedLines) - v->numAttachedLines);
    }

    if(line->b && line->b->numAttachedLines > 0)
    {
        struct MapVertex *v = line->b;
        for(size_t i = line->bVertIndex + 1; i < v->numAttachedLines; ++i)
        {
            struct MapLine *attLine = v->attachedLines[i];
            attLine->a == v ? attLine->aVertIndex-- : attLine->bVertIndex--;
        }
        memmove(v->attachedLines + line->bVertIndex, v->attachedLines + line->bVertIndex + 1, (v->numAttachedLines - (line->bVertIndex)) * sizeof *v->attachedLines);
        v->numAttachedLines--;
        //memset(v->attachedLines + v->numAttachedLines, 0, COUNT_OF(v->attachedLines) - v->numAttachedLines);
    }

    FreeMapLine(line);

    map->numLines--;
    map->dirty = true;
}

static struct MapLine* FindLine(struct EdState state[static 1], vec2s a, vec2s b)
{
    struct Map *map = &state->map;
    for(struct MapLine *line = map->headLine; line; line = line->next)
    {
        vec2s la = line->a->pos, lb = line->b->pos;
        if((glms_vec2_eqv_eps(la, a) && glms_vec2_eqv_eps(lb, b)) || (glms_vec2_eqv_eps(la, b) && glms_vec2_eqv_eps(lb, a))) return line;
    }
    return NULL;
}

static struct MapSector* AddPolygon(struct EdState state[static 1], struct Polygon polygon[static 1])
{
    struct Map *map = &state->map;
    assert(polygon);

    static size_t sectorIndex = 0;

    struct MapSector *sector = calloc(1, sizeof *sector);
    sector->numOuterLines = polygon->length;
    sector->outerLines = malloc(sector->numOuterLines * sizeof *sector->outerLines);
    sector->idx = sectorIndex++;
    sector->prev = map->tailSector;

    if(map->headSector == NULL)
    {
        map->headSector = sector;
        map->tailSector = sector;
    }
    else if(map->tailSector->prev == NULL)
    {
        map->headSector->next = sector;
    }
    else
    {
        map->tailSector->next = sector;
    }
    map->tailSector = sector;

    sector->vertices = calloc(polygon->length, sizeof *sector->vertices);
    memcpy(sector->vertices, polygon->vertices, polygon->length * sizeof *polygon->vertices);

    for(size_t i = 0; i < polygon->length; ++i)
    {
        size_t inext = (i + 1) % polygon->length;
        vec2s a = { .x = polygon->vertices[i][0], .y = polygon->vertices[i][1] };
        vec2s b = { .x = polygon->vertices[inext][0], .y = polygon->vertices[inext][1] };
        struct MapLine *line = FindLine(state, a, b);
        if(line == NULL)
        {
            struct MapVertex *va = EditAddVertex(state, a);
            struct MapVertex *vb = EditAddVertex(state, b);
            line = EditAddLine(state, va, vb);
        }

        sector->outerLines[i] = line;
    }

    size_t baseVertexIndex = state->gl.editorSector.highestVertIndex;
    size_t baseIndexIndex = state->gl.editorSector.highestIndIndex;

    size_t index = baseVertexIndex;
    for(size_t i = 0; i < polygon->length; ++i)
    {
        int32_t x = polygon->vertices[i][0];
        int32_t y = polygon->vertices[i][1];
        state->gl.editorSector.bufferMap[index++] = (struct SectorVertexType){ .position = {{ x, y }}, .color = { 1, 1, 1, 1 }, .texCoord = {{ 0, 0 }} };
    }
    unsigned int *indices = NULL;
    unsigned long numIndices = triangulate(polygon, NULL, 0, &indices);

    index = baseIndexIndex;
    for(size_t i = 0; i < numIndices; ++i)
        state->gl.editorSector.indexMap[index++] = indices[i] + baseVertexIndex;

    free(indices);

    sector->edData = (struct TriangleData){ .indexStart = baseIndexIndex, .indexLength = numIndices, .vertexStart = baseVertexIndex, .vertexLength = polygon->length };

    state->gl.editorSector.highestVertIndex += polygon->length;
    state->gl.editorSector.highestIndIndex += numIndices;

    map->dirty = true;
    return sector;
}

static struct Polygon* PolygonFromSector(struct Map map[static 1], struct MapSector sector[static 1])
{
    struct Polygon *polygon = calloc(1, sizeof *polygon + sector->numOuterLines * sizeof *polygon->vertices);
    polygon->length = sector->numOuterLines;
    memcpy(polygon->vertices, sector->vertices, polygon->length * sizeof *sector->vertices);
    return polygon;
}

static void SplitMapLine(struct EdState state[static 1], struct MapLine line[static 1], struct MapVertex vertex[static 1])
{
    struct Map *map = &state->map;

    struct Side frontCopy = line->front;
    struct Side backCopy = line->back;

    struct MapVertex *va = line->a;
    struct MapVertex *vb = line->b;

    RemoveLine(map, line);

    struct MapLine *newA = EditAddLine(state, va, vertex);
    struct MapLine *newB = EditAddLine(state, vertex, vb);

    newA->front = frontCopy;
    newA->back = backCopy;
    newB->front = frontCopy;
    newB->back = backCopy;
}

static void SplitMapLine2(struct EdState state[static 1], struct MapLine line[static 1], struct MapVertex vertexA[static 1], struct MapVertex vertexB[static 1])
{
    struct Map *map = &state->map;

    struct Side frontCopy = line->front;
    struct Side backCopy = line->back;

    struct MapVertex *va = line->a;
    struct MapVertex *vb = line->b;

    RemoveLine(map, line);

    struct MapLine *newStart = EditAddLine(state, va, vertexA);
    struct MapLine *newMiddle = EditAddLine(state, vertexA, vertexB);
    struct MapLine *newEnd = EditAddLine(state, vertexB, vb);

    newStart->front = frontCopy;
    newStart->back = backCopy;
    newMiddle->front = frontCopy;
    newMiddle->back = backCopy;
    newEnd->front = frontCopy;
    newEnd->back = backCopy;
}

static bool InsertLinesIntoMap(struct EdState state[static 1], size_t numVerts, vec2s vertices[static numVerts], bool isLoop)
{
    struct Map *map = &state->map;
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

    //constexpr size_t QUEUE_SIZE = 1<<12;
#define QUEUE_SIZE (1<<12)

    struct
    {
        struct line_t lines[QUEUE_SIZE];
        int head, tail, numLines;
    } queue = { 0 };

    // insert the drawn lines into queue
    for(size_t i = 0; i < end; ++i)
    {
        vec2s a = vertices[i];
        vec2s b = vertices[(i+1) % numVerts];

        queue.lines[queue.tail] = (struct line_t){ .a = a, .b = b };
        queue.tail = (queue.tail + 1) % QUEUE_SIZE;
        queue.numLines++;

        assert(queue.tail != queue.head);
    }

    while(queue.numLines > 0)
    {
        struct line_t line = queue.lines[queue.head];
        queue.head = (queue.head + 1) % QUEUE_SIZE;
        queue.numLines--;

        bool canInsertLine = true;
        bool intersect;
        struct MapLine *mapLine = map->headLine;
        while(mapLine != NULL)
        {
            struct line_t mline = { .a = mapLine->a->pos, .b = mapLine->b->pos };
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
                    struct line_t major = { .a = commonPoint, .b = glms_vec2_eqv_eps(commonPoint, mline.a) ? mline.b : mline.a };
                    struct line_t minor = { .a = commonPoint, .b = glms_vec2_eqv_eps(commonPoint, line.a) ? line.b : line.a };

                    vec2s u = glms_vec2_sub(major.b, major.a);
                    vec2s v = glms_vec2_sub(minor.b, minor.a);
                    float dot = glms_vec2_dot(u, v);
                    if(dot > 0) // they do infact overlap
                    {
                        float t = LineGetPointFactor(major, minor.b);
                        if(t > 1)
                        {
                            queue.lines[queue.tail] = (struct line_t){ .a = major.b, .b = minor.b };
                            queue.tail = (queue.tail + 1) % QUEUE_SIZE;
                            queue.numLines++;

                            assert(queue.numLines < QUEUE_SIZE);
                        }
                        else
                        {
                            struct MapVertex *splitVertex = EditAddVertex(state, minor.b);
                            struct MapLine *lineToSplit = mapLine;
                            mapLine = NULL; // since we are splitting the line here we should stop iterating through the rest of the map lines
                            SplitMapLine(state, lineToSplit, splitVertex);
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

            struct line_t lineCorrected = line;
            if(signbit(mlOrient) != signbit(lOrient))
            {
                lineCorrected.a = line.b;
                lineCorrected.b = line.a;
            }

            struct intersection_res_t result = {0};
            if((intersect = LineIntersection(mline, line, &result)))
            {
                if(!glms_vec2_eqv_eps(mline.a, result.p0) && !glms_vec2_eqv_eps(mline.b, result.p0))
                {
                    struct MapVertex *splitVertex = EditAddVertex(state, result.p0);
                    struct MapLine *lineToSplit = mapLine;
                    SplitMapLine(state, lineToSplit, splitVertex);
                }

                if(!glms_vec2_eqv_eps(line.a, result.p0))
                {
                    queue.lines[queue.tail] = (struct line_t){ .a = line.a, .b = result.p0 };
                    queue.tail = (queue.tail + 1) % QUEUE_SIZE;
                    queue.numLines++;
                }

                if(!glms_vec2_eqv_eps(result.p0, line.b))
                {
                    queue.lines[queue.tail] = (struct line_t){ .a = result.p0, .b = line.b };
                    queue.tail = (queue.tail + 1) % QUEUE_SIZE;
                    queue.numLines++;
                }

                mapLine = NULL; // since we are splitting the line here we should stop iterating through the rest of the map lines

                assert(queue.numLines < QUEUE_SIZE);
            }
            else if((intersect = LineOverlap(mline, lineCorrected, &result)))
            {
                if(result.t0 == 0 && result.t1 == 1)
                {
                    struct MapVertex *splitVertexA = EditAddVertex(state, result.p0);
                    struct MapVertex *splitVertexB = EditAddVertex(state, result.p1);
                    struct MapLine *lineToSplit = mapLine;
                    mapLine = NULL; // since we are splitting the line here we should stop iterating through the rest of the map lines
                    SplitMapLine2(state, lineToSplit, splitVertexA, splitVertexB);
                }
                else if(result.t0 == 0)
                {
                    struct MapVertex *splitVertex = EditAddVertex(state, result.p0);
                    struct MapLine *lineToSplit = mapLine;
                    mapLine = NULL; // since we are splitting the line here we should stop iterating through the rest of the map lines
                    SplitMapLine(state, lineToSplit, splitVertex);

                    queue.lines[queue.tail] = (struct line_t){ .a = mline.b, .b = lineCorrected.b };
                    queue.tail = (queue.tail + 1) % QUEUE_SIZE;
                    queue.numLines++;

                    assert(queue.numLines < QUEUE_SIZE);
                }
                else if(result.t1 == 0)
                {
                    struct MapVertex *splitVertex = EditAddVertex(state, result.p1);
                    struct MapLine *lineToSplit = mapLine;
                    mapLine = NULL; // since we are splitting the line here we should stop iterating through the rest of the map lines
                    SplitMapLine(state, lineToSplit, splitVertex);

                    queue.lines[queue.tail] = (struct line_t){ .a = mline.a, .b = lineCorrected.a };
                    queue.tail = (queue.tail + 1) % QUEUE_SIZE;
                    queue.numLines++;

                    assert(queue.numLines < QUEUE_SIZE);
                }
                else
                {
                    queue.lines[queue.tail] = (struct line_t){ .a = mline.b, .b = lineCorrected.b };
                    queue.tail = (queue.tail + 1) % QUEUE_SIZE;
                    queue.numLines++;

                    queue.lines[queue.tail] = (struct line_t){ .a = lineCorrected.a, .b = mline.a };
                    queue.tail = (queue.tail + 1) % QUEUE_SIZE;
                    queue.numLines++;

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
            struct MapVertex *mva = EditAddVertex(state, line.a);
            struct MapVertex *mvb = EditAddVertex(state, line.b);
            if(!mva || !mvb) return false;

            struct MapLine *line = EditAddLine(state, mva, mvb);
            if(!line) return false;
        }

        didIntersect |= !canInsertLine;
    }

    // create sectors from the new lines
    if(isLoop)
    {

    }
    else if(didIntersect) // not a loop but lines did intersect: update sectors from the touched lines
    {

    }

    return true;
}

#if 0
static void RemoveSector(struct EdState *state, struct MapSector *sector)
{
    struct Map *map = &state->map;
    assert(sector);

    struct MapSector *prev = sector->prev;
    struct MapSector *next = sector->next;

    for(struct MapSector *s = next; s; s = s->next)
    {
        s->idx--;
    }

    if(prev == NULL && next == NULL)
    {
        map->headSector = map->tailSector = NULL;
    }
    else if(sector == map->headSector)
    {
        next->prev = NULL;
        map->headSector = next;
    }
    else if(sector == map->tailSector)
    {
        prev->next = NULL;
        map->tailSector = prev;
    }
    else
    {
        prev->next = next;
        next->prev = prev;
    }

    /* dont remove outer lines, only the sector
    for(size_t i = 0; i < sector->numOuterLines; ++i)
    {
        RemoveLine(state, sector->outerLines[i]);
    }
    */

    // do decrement the ref count for inner lines
    for(size_t i = 0; i < sector->numInnerLines; ++i)
    {
        for(size_t j = 0; j < sector->numInnerLinesNum[i]; ++j)
        {
            RemoveLine(state, sector->innerLines[i][j]);
        }
    }

    pstr_free(sector->ceilTex);
    pstr_free(sector->floorTex);
    free(sector->outerLines);
    for(size_t i = 0; i < sector->numInnerLines; ++i)
        free(sector->innerLines[i]);
    free(sector->innerLines);
    free(sector);

    map->numSectors--;

    map->dirty = true;
}
#endif

void ScreenToEditorSpace(const struct EdState state[static 1], int32_t x[static 1], int32_t y[static 1])
{
    const float z = state->data.zoomLevel;
    *x = (int32_t)((*x + state->data.viewPosition.x) / z);
    *y = (int32_t)((*y + state->data.viewPosition.y) / z);
}

void ScreenToEditorSpacef(const struct EdState state[static 1], float x[static 1], float y[static 1])
{
    const float z = state->data.zoomLevel;
    *x = (*x + state->data.viewPosition.x) / z;
    *y = (*y + state->data.viewPosition.y) / z;
}

void EditorToScreenSpace(const struct EdState state[static 1], int32_t x[static 1], int32_t y[static 1])
{
    const float z = state->data.zoomLevel;
    *x = (int32_t)((*x - state->data.viewPosition.x) * z);
    *y = (int32_t)((*y - state->data.viewPosition.y) * z);
}

void ScreenToEditorSpaceGrid(const struct EdState state[static 1], int32_t x[static 1], int32_t y[static 1])
{
    ScreenToEditorSpace(state, x, y);
    const int offset = state->data.gridSize / 2;
    int32_t xt = *x, yt = *y;
    xt += xt < 0 ? -offset : offset;
    *x = xt / state->data.gridSize * state->data.gridSize;
    yt += yt < 0 ? -offset : offset;
    *y = yt / state->data.gridSize * state->data.gridSize;
}

void EditCopy(struct EdState state[static 1])
{
    LogDebug("Copy!!\n");
}

void EditPaste(struct EdState state[static 1])
{
    LogDebug("Paste!!\n");
}

void EditCut(struct EdState state[static 1])
{
    LogDebug("Cut!!\n");
}

struct MapVertex* EditAddVertex(struct EdState state[static 1], vec2s pos)
{
    struct Map *map = &state->map;
    for(struct MapVertex *vertex = map->headVertex; vertex; vertex = vertex->next)
    {
        if(glms_vec2_eqv_eps(vertex->pos, pos)) 
        {
            return vertex;
        }
    }

    static size_t vertexIndex = 0;

    struct MapVertex *vertex = calloc(1, sizeof *vertex);
    vertex->pos = pos;
    vertex->idx = vertexIndex++;
    vertex->prev = map->tailVertex;

    if(map->headVertex == NULL)
    {
        map->headVertex = vertex;
        map->tailVertex = vertex;
    }
    else if(map->tailVertex->prev == NULL)
    {
        map->headVertex->next = vertex;
    }
    else
    {
        map->tailVertex->next = vertex;
    }
    map->tailVertex = vertex;

    state->gl.editorVertex.bufferMap[vertex->idx] = (struct VertexType){ .position = pos, .color = { 1, 1, 1, 1 } };

    map->dirty = true;
    return vertex;
}

void EditRemoveVertices(struct EdState state[static 1], size_t num, struct MapVertex *vertices[static num])
{
    struct Map *map = &state->map;
    
    struct MapLine *potentialLines[4096] = { 0 };
    size_t numPotentialLines = 0;

    for(size_t i = 0; i < num; ++i)
    {
        struct MapVertex *vertex = vertices[i];
        for(size_t i = 0; i < vertex->numAttachedLines; ++i)
        {
            bool lineIsInSet = false;
            struct MapLine *attLine = vertex->attachedLines[i];
            for(size_t j = 0; j < numPotentialLines; ++j)
            {
                lineIsInSet |= potentialLines[j] == attLine;
                if(lineIsInSet) break;
            }

            if(!lineIsInSet) potentialLines[numPotentialLines++] = attLine;
        }
        RemoveVertex(map, vertex);
    }

    for(size_t i = 0; i < numPotentialLines; ++i)
    {
        struct MapLine *line = potentialLines[i];
        if(!line->a || !line->b)
            RemoveLine(map, line);
    }

    map->dirty = true;
}

struct MapVertex* EditGetVertex(struct EdState state[static 1], vec2s pos)
{
    struct Map *map = &state->map;
    for(struct MapVertex *vertex = map->headVertex; vertex; vertex = vertex->next)
    {
        if(vertex->pos.x == pos.x && vertex->pos.y == pos.y)
        {
            return vertex;
        }
    }
    return NULL;
}

struct MapVertex* EditGetClosestVertex(struct EdState state[static 1], vec2s pos, float maxDist)
{
    struct Map *map = &state->map;
    struct MapVertex *closestVertex = NULL;
    float closestDist = 100000;
    for(struct MapVertex *vertex = map->headVertex; vertex; vertex = vertex->next)
    {
        float dist2 = glms_vec2_distance2(vertex->pos, pos);
        if(dist2 <= maxDist*maxDist)
        {
            float dist = sqrt(dist2);
            if(dist < closestDist)
            {
                closestDist = dist;
                closestVertex = vertex;
            }
        }
    }
    return closestVertex;
}

struct MapLine* EditAddLine(struct EdState state[static 1], struct MapVertex v0[static 1], struct MapVertex v1[static 1])
{
    struct Map *map = &state->map;

    for(struct MapLine *line = map->headLine; line; line = line->next)
    {
        bool ab = line->a == v0 && line->b == v1;
        bool ba = line->a == v1 && line->b == v0;
        if(ab || ba) 
        {
            return line;
        }
    }

    const vec2s vert0 = v0->pos;
    const vec2s vert1 = v1->pos;
    int32_t normal = sign((vert0.x*vert1.y) - (vert0.y*vert1.x));

    static size_t lineIndex = 0;

    struct MapLine *line = calloc(1, sizeof *line);
    line->a = v0;
    line->b = v1;
    line->normal = normal;
    line->idx = lineIndex++;
    line->prev = map->tailLine;

    v0->attachedLines[v0->numAttachedLines] = line;
    line->aVertIndex = v0->numAttachedLines;
    v0->numAttachedLines++;
    v1->attachedLines[v1->numAttachedLines] = line;
    line->bVertIndex = v1->numAttachedLines;
    v1->numAttachedLines++;

    if(map->headLine == NULL)
    {
        map->headLine = line;
        map->tailLine = line;
    }
    else if(map->tailLine->prev == NULL)
    {
        map->headLine->next = line;
    }
    else
    {
        map->tailLine->next = line;
    }
    map->tailLine = line;

    vec2s l = glms_vec2_sub(vert1, vert0);
    vec2s n = {{ -l.y, l.x }};
    n = glms_vec2_normalize(n);

    vec2s middle = glms_vec2_scale(l, 0.5f);
    middle = glms_vec2_add(vert0, middle);

    vec2s middleNormal = glms_vec2_scale(n, 10.0f);
    middleNormal = glms_vec2_add(middle, middleNormal);

    state->gl.editorLine.bufferMap[line->idx * 4    ] = (struct VertexType){ .position = vert0, .color = { 1, 1, 1, 1 } };
    state->gl.editorLine.bufferMap[line->idx * 4 + 1] = (struct VertexType){ .position = vert1, .color = { 1, 1, 1, 1 } };
    state->gl.editorLine.bufferMap[line->idx * 4 + 2] = (struct VertexType){ .position = middle, .color = { 1, 1, 1, 1 } };
    state->gl.editorLine.bufferMap[line->idx * 4 + 3] = (struct VertexType){ .position = middleNormal, .color = { 1, 1, 1, 1 } };

    map->dirty = true;
    return line;
}

void EditRemoveLines(struct EdState state[static 1], size_t num, struct MapLine *lines[static num])
{
    struct Map *map = &state->map;

    struct MapVertex *potentialVertices[4096] = { 0 };
    size_t numPotentialVertices = 0;

    for(size_t i = 0; i < num; ++i)
    {
        struct MapLine *line = lines[i];
        bool isAInSet = false;
        bool isBInSet = false;
        for(size_t i = 0; i < numPotentialVertices; ++i)
        {
            if(!isAInSet) isAInSet = potentialVertices[i] == line->a;
            if(!isBInSet) isBInSet = potentialVertices[i] == line->b;
            if(isAInSet && isBInSet) break;
        }
        if(!isAInSet) potentialVertices[numPotentialVertices++] = line->a;
        if(!isBInSet) potentialVertices[numPotentialVertices++] = line->b;

        RemoveLine(map, line);
    }

    for(size_t i = 0; i < numPotentialVertices; ++i)
    {
        struct MapVertex *vertex = potentialVertices[i];
        if(vertex->numAttachedLines == 0) RemoveVertex(map, vertex);
    }

    map->dirty = true;
}

struct MapLine* EditGetClosestLine(struct EdState state[static 1], vec2s pos, float maxDist)
{
    struct Map *map = &state->map;
    struct MapLine *closestLine = NULL;
    float closestDist = 10000;
    for(struct MapLine *line = map->headLine; line; line = line->next)
    {
        float dist = MinDistToLine(line->a->pos, line->b->pos, pos);
        if(dist <= maxDist && dist < closestDist)
        {
            closestDist = dist;
            closestLine = line;
        }
    }
    return closestLine;
}

void EditRemoveSectors(struct EdState state[static 1], size_t num, struct MapSector *sectors[static num])
{
    struct Map *map = &state->map;

    for(size_t i = 0; i < num; ++i)
    {
        struct MapSector *sector = sectors[i];

        struct MapSector *prev = sector->prev;
        struct MapSector *next = sector->next;

        for(struct MapSector *s = next; s; s = s->next)
        {
            s->idx--;
        }

        if(prev == NULL && next == NULL)
        {
            map->headSector = map->tailSector = NULL;
        }
        else if(sector == map->headSector)
        {
            next->prev = NULL;
            map->headSector = next;
        }
        else if(sector == map->tailSector)
        {
            prev->next = NULL;
            map->tailSector = prev;
        }
        else
        {
            prev->next = next;
            next->prev = prev;
        }

        for(size_t i = 0; i < sector->numOuterLines; ++i)
        {
            // RemoveLine(state, sector->outerLines[i]);
            struct MapLine *line = sector->outerLines[i];
            if(line->frontSector == sector) line->frontSector = NULL;
            else if(line->backSector == sector) line->backSector = NULL;
        }

        /*
        for(size_t i = 0; i < sector->numInnerLines; ++i)
        {
            for(size_t j = 0; j < sector->numInnerLinesNum[i]; ++j)
            {
                RemoveLine(state, sector->innerLines[i][j]);
            }
        }
        */
        FreeMapSector(sector);

        map->numSectors--;
    }

    map->dirty = true;
}

struct MapSector* EditGetSector(struct EdState state[static 1], vec2s pos)
{
    struct Map *map = &state->map;

    for(struct MapSector *sector = map->headSector; sector; sector = sector->next)
    {
        struct 
        {
            struct MapSector *s[1000];
            int top;
        } stack = { .s = { sector }, .top = 0 };

        while(stack.top >= 0)
        {
            struct MapSector *sectorToCheck = stack.s[stack.top--];
            if(sectorToCheck->numContains > 0)
            {
                stack.s[++stack.top] = sectorToCheck;
                for(size_t i = 0; i < sectorToCheck->numContains; ++i)
                {
                    stack.s[++stack.top] = sectorToCheck->contains[i];
                }
                continue;
            }

            if(PointInSector(sectorToCheck, pos))
                return sectorToCheck;
        }
    }

    return NULL;
}

struct MapLine* EditApplyLines(struct EdState state[static 1], size_t num, vec2s points[static num])
{
    InsertLinesIntoMap(state, num, points, false);
    return NULL;
}

struct MapSector* EditApplySector(struct EdState state[static 1], size_t num, vec2s points[static num])
{
    InsertLinesIntoMap(state, num, points, true);
    return NULL;
}
