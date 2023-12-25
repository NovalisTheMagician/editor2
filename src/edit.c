#include "edit.h"
#include "map.h"

#include "geometry.h"

#include <assert.h>
#include <tgmath.h>

#include <triangulate.h>

static void RemoveVertex(struct Map *map, struct MapVertex *vertex)
{
    assert(vertex);

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

    free(vertex);

    map->numVertices--;
    map->dirty = true;
}

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

static void RemoveLineSimple(struct Map map[static 1], struct MapLine *line)
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
    
    for(size_t i = line->aVertIndex + 1; i < line->a->numAttachedLines; ++i)
    {
        struct MapLine *attLine = line->a->attachedLines[i];
        attLine->a == line->a ? attLine->aVertIndex-- : attLine->bVertIndex--;
    }
    memmove(line->a->attachedLines + line->aVertIndex, line->a->attachedLines + line->aVertIndex + 1, ((line->a->numAttachedLines - (line->aVertIndex)) * sizeof *line->a->attachedLines));
    line->a->numAttachedLines--;

    for(size_t i = line->bVertIndex + 1; i < line->b->numAttachedLines; ++i)
    {
        struct MapLine *attLine = line->b->attachedLines[i];
        attLine->a == line->b ? attLine->aVertIndex-- : attLine->bVertIndex--;
    }
    memmove(line->b->attachedLines + line->bVertIndex, line->b->attachedLines + line->bVertIndex + 1, ((line->b->numAttachedLines - (line->bVertIndex)) * sizeof *line->b->attachedLines));
    line->b->numAttachedLines--;

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

static inline bool VertexCmp(ivec2s a, ivec2s b)
{
    return a.x == b.x && a.y == b.x;
}

static struct MapLine* FindLine(struct EdState *state, ivec2s a, ivec2s b)
{
    struct Map *map = &state->map;
    for(struct MapLine *line = map->headLine; line; line = line->next)
    {
        ivec2s la = line->a->pos, lb = line->b->pos;
        if((VertexCmp(la, a) && VertexCmp(lb, b)) || (VertexCmp(la, b) && VertexCmp(lb, a))) return line;
    }
    return NULL;
}

static struct MapSector* AddPolygon(struct EdState *state, struct Polygon *polygon)
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
        ivec2s a = { .x = polygon->vertices[i][0], .y = polygon->vertices[i][1] };
        ivec2s b = { .x = polygon->vertices[inext][0], .y = polygon->vertices[inext][1] };
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
        state->gl.editorSector.bufferMap[index++] = (struct SectorVertexType){ .position = { x, y }, .color = { 1, 1, 1, 1 }, .texCoord = { 0, 0 } };
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

static struct Polygon* PolygonFromSector(struct Map *map, struct MapSector *sector)
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

    RemoveLineSimple(map, line);

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

    RemoveLineSimple(map, line);

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

static bool InsertLinesIntoMap(struct EdState state[static 1], size_t numVerts, ivec2s vertices[static numVerts], bool isLoop)
{
    struct Map *map = &state->map;
    bool didIntersect = false;
    size_t end = isLoop ? numVerts : numVerts - 1;

    //constexpr size_t QUEUE_SIZE = 1000;
#define QUEUE_SIZE 1000

    struct
    {
        struct line_t lines[QUEUE_SIZE];
        int head, tail, numLines;
    } queue = { 0 };

    for(size_t i = 0; i < end; ++i)
    {
        ivec2s a = vertices[i];
        ivec2s b = vertices[(i+1) % numVerts];

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
        enum intersection_type_t intersect;
        struct MapLine *mapLine = map->headLine;
        while(mapLine != NULL)
        {
            struct intersection_res_t result = {0};
            intersect = LineIntersection((struct line_t){ .a = mapLine->a->pos, .b = mapLine->b->pos }, line, &result);

            if(intersect == INTERSECTION)
            {
                struct MapVertex *splitVertex = EditAddVertex(state, result.p0);
                struct MapLine *lineToSplit = mapLine;
                mapLine = NULL; // since we are splitting the line here we should stop iterating through the rest of the lines
                SplitMapLine(state, lineToSplit, splitVertex);

                queue.lines[queue.tail] = (struct line_t){ .a = line.a, .b = result.p0 };
                queue.tail = (queue.tail + 1) % QUEUE_SIZE;
                queue.numLines++;

                queue.lines[queue.tail] = (struct line_t){ .a = result.p0, .b = line.b };
                queue.tail = (queue.tail + 1) % QUEUE_SIZE;
                queue.numLines++;
                
                assert(queue.numLines < QUEUE_SIZE);

                LogDebug("Intersection!");
            }
            else if(intersect == OVERLAP) // overlap
            {
                struct MapVertex *splitVertexA = EditAddVertex(state, result.p0);
                struct MapVertex *splitVertexB = EditAddVertex(state, result.p1);
                struct MapLine *lineToSplit = mapLine;
                mapLine = NULL; // since we are splitting the line here we should stop iterating through the rest of the lines
                SplitMapLine2(state, lineToSplit, splitVertexA, splitVertexB);

                LogDebug("Overlap!");
            }
            else
                mapLine = mapLine->next;
            canInsertLine &= intersect == NO_INTERSECTION;
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

void ScreenToEditorSpace(const struct EdState *state, int32_t *x, int32_t *y)
{
    const float z = state->data.zoomLevel;
    *x = (int32_t)((*x + state->data.viewPosition.x) / z);
    *y = (int32_t)((*y + state->data.viewPosition.y) / z);
}

void ScreenToEditorSpacef(const struct EdState *state, float *x, float *y)
{
    const float z = state->data.zoomLevel;
    *x = (*x + state->data.viewPosition.x) / z;
    *y = (*y + state->data.viewPosition.y) / z;
}

void EditorToScreenSpace(const struct EdState *state, int32_t *x, int32_t *y)
{
    const float z = state->data.zoomLevel;
    *x = (int32_t)((*x - state->data.viewPosition.x) * z);
    *y = (int32_t)((*y - state->data.viewPosition.y) * z);
}

void ScreenToEditorSpaceGrid(const struct EdState *state, int32_t *x, int32_t *y)
{
    ScreenToEditorSpace(state, x, y);
    const int offset = state->data.gridSize / 2;
    int32_t xt = *x, yt = *y;
    xt += xt < 0 ? -offset : offset;
    *x = xt / state->data.gridSize * state->data.gridSize;
    yt += yt < 0 ? -offset : offset;
    *y = yt / state->data.gridSize * state->data.gridSize;
}

void EditCopy(struct EdState *state)
{
    LogDebug("Copy!!\n");
}

void EditPaste(struct EdState *state)
{
    LogDebug("Paste!!\n");
}

void EditCut(struct EdState *state)
{
    LogDebug("Cut!!\n");
}

struct MapVertex* EditAddVertex(struct EdState *state, ivec2s pos)
{
    struct Map *map = &state->map;
    for(struct MapVertex *vertex = map->headVertex; vertex; vertex = vertex->next)
    {
        if(vertex->pos.x == pos.x && vertex->pos.y == pos.y) 
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

    state->gl.editorVertex.bufferMap[vertex->idx] = (struct VertexType){ .position = {{ pos.x, pos.y }}, .color = { 1, 1, 1, 1 } };

    map->dirty = true;
    return vertex;
}

void EditRemoveVertex(struct EdState *state, struct MapVertex *vertex)
{
    assert(vertex);
    struct Map *map = &state->map;
    
    for(struct MapLine *line = map->headLine; line; line = line->next)
    {
        if(line->a == vertex || line->b == vertex)
        {
            LogWarning("Can't delete vertex! Part of line.");
            return;
        }
    }

    RemoveVertex(map, vertex);

    map->dirty = true;
}

struct MapVertex* EditGetVertex(struct EdState *state, ivec2s pos)
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

struct MapVertex* EditGetClosestVertex(struct EdState *state, ivec2s pos, float maxDist)
{
    struct Map *map = &state->map;
    struct MapVertex *closestVertex = NULL;
    float closestDist = 100000;
    for(struct MapVertex *vertex = map->headVertex; vertex; vertex = vertex->next)
    {
        float dist2 = dist2(vertex->pos, pos);
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

struct MapLine* EditAddLine(struct EdState *state, struct MapVertex *v0, struct MapVertex *v1)
{
    struct Map *map = &state->map;
    assert(v0);
    assert(v1);

    for(struct MapLine *line = map->headLine; line; line = line->next)
    {
        bool ab = line->a == v0 && line->b == v1;
        bool ba = line->a == v1 && line->b == v0;
        if(ab || ba) 
        {
            return line;
        }
    }

    const ivec2s vert0 = v0->pos;
    const ivec2s vert1 = v1->pos;
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

    vec2s l = glms_vec2_sub((vec2s){{ vert1.x, vert1.y }}, (vec2s){{ vert0.x, vert0.y }});
    vec2s n = {{ -l.y, l.x }};
    n = glms_vec2_normalize(n);

    vec2s middle = glms_vec2_scale(l, 0.5f);
    middle = glms_vec2_add((vec2s){{ vert0.x, vert0.y }}, middle);

    vec2s middleNormal = glms_vec2_scale(n, 10.0f);
    middleNormal = glms_vec2_add(middle, middleNormal);

    state->gl.editorLine.bufferMap[line->idx * 4    ] = (struct VertexType){ .position = {{ vert0.x, vert0.y }}, .color = { 1, 1, 1, 1 } };
    state->gl.editorLine.bufferMap[line->idx * 4 + 1] = (struct VertexType){ .position = {{ vert1.x, vert1.y }}, .color = { 1, 1, 1, 1 } };
    state->gl.editorLine.bufferMap[line->idx * 4 + 2] = (struct VertexType){ .position = {{ middle.x, middle.y }}, .color = { 1, 1, 1, 1 } };
    state->gl.editorLine.bufferMap[line->idx * 4 + 3] = (struct VertexType){ .position = {{ middleNormal.x, middleNormal.y }}, .color = { 1, 1, 1, 1 } };

    map->dirty = true;
    return line;
}

void EditRemoveLine(struct EdState *state, struct MapLine *line)
{
    assert(line);
    struct Map *map = &state->map;

    for(struct MapSector *sector = map->headSector; sector; sector = sector->next)
    {
        for(size_t i = 0; i < sector->numOuterLines; ++i)
        {
            if(sector->outerLines[i] == line)
            {
                LogWarning("Can't remove line! Part of sector");
                return;
            }
        }
    }

    RemoveLine(map, line);

    map->dirty = true;
}

struct MapLine* EditGetClosestLine(struct EdState *state, ivec2s pos, float maxDist)
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

void EditRemoveSector(struct EdState *state, struct MapSector *sector)
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

struct MapSector* EditGetSector(struct EdState *state, ivec2s pos)
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

struct MapLine* EditApplyLines(struct EdState *state, ivec2s *points, size_t num)
{
    InsertLinesIntoMap(state, num, points, false);
    return NULL;
}

struct MapSector* EditApplySector(struct EdState *state, ivec2s *points, size_t num)
{
    InsertLinesIntoMap(state, num, points, true);
    return NULL;
}
