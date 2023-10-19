#include "edit.h"
#include "map.h"

#include <assert.h>
#include <tgmath.h>

#include <triangulate.h>

#define dot(a, b) ({ struct Vertex a_ = (a); struct Vertex b_ = (b); a_.x * b_.x + a_.y * b_.y; })
#define dist2(a, b) ({ struct Vertex a_ = (a); struct Vertex b_ = (b); float dx = a_.x - b_.x; float dy = a_.y - b_.y; dx*dx + dy*dy; })
#define between(p, a, b) ({ __typeof__(p) p_ = (p); __typeof__(a) a_ = (a); __typeof__(b) b_ = (b); (p_ >= a_ && p_ <= b_) || (p_ <= a_ && p_ >= b_);})

static int32_t sign(int32_t x) {
    return (x > 0) - (x < 0);
}

static void RemoveVertex(struct EdState *state, struct MapVertex *vertex, bool force)
{
    struct Map *map = &state->map;
    assert(vertex);
    assert(vertex->refCount > 0);

    vertex->refCount--;
    if(vertex->refCount == 0 || force)
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

        free(vertex);

        map->numVertices--;
        map->dirty = true;
    }
}

static void RemoveLine(struct EdState *state, struct MapLine *line)
{
    struct Map *map = &state->map;
    assert(line);
    assert(line->refCount > 0);

    line->refCount--;
    if(line->refCount == 0)
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

        RemoveVertex(state, line->a, false);
        RemoveVertex(state, line->b, false);

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
}

static inline bool VertexCmp(struct Vertex a, struct Vertex b)
{
    return a.x == b.x && a.y == b.x;
}

static struct MapLine* FindLine(struct EdState *state, struct Vertex a, struct Vertex b)
{
    struct Map *map = &state->map;
    for(struct MapLine *line = map->headLine; line; line = line->next)
    {
        struct Vertex la = line->a->pos, lb = line->b->pos;
        if((VertexCmp(la, a) && VertexCmp(lb, b)) || (VertexCmp(la, b) && VertexCmp(lb, a))) return line;
    }
    return NULL;
}

static struct MapVertex* FindVertex(struct EdState *state, struct Vertex v)
{
    struct Map *map = &state->map;
    for(struct MapVertex *vertex = map->headVertex; vertex; vertex = vertex->next)
    {
        struct Vertex la = vertex->pos;
        if(VertexCmp(la, v)) return vertex;
    }
    return NULL;
}

static inline float MinDistToLine(struct Vertex v, struct Vertex w, struct Vertex p)
{
    float l2 = dist2(v, w);
    if (l2 == 0) return dist2(p, v);
    float t = ((p.x - v.x) * (w.x - v.x) + (p.y - v.y) * (w.y - v.y)) / l2;
    t = max(0, min(1, t));
    struct Vertex tmp = { .x = v.x + t * (w.x - v.x), .y = v.y + t * (w.y - v.y) };
    return sqrt(dist2(p, tmp));
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
        struct Vertex a = { .x = polygon->vertices[i][0], .y = polygon->vertices[i][1] };
        struct Vertex b = { .x = polygon->vertices[inext][0], .y = polygon->vertices[inext][1] };
        struct MapLine *line = FindLine(state, a, b);
        if(line == NULL)
        {
            struct MapVertex *va = EditAddVertex(state, a);
            struct MapVertex *vb = EditAddVertex(state, b);
            line = EditAddLine(state, va, vb);
        }
        else
        {
            line->refCount++;
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

struct MapVertex* EditAddVertex(struct EdState *state, struct Vertex pos)
{
    struct Map *map = &state->map;
    for(struct MapVertex *vertex = map->headVertex; vertex; vertex = vertex->next)
    {
        if(vertex->pos.x == pos.x && vertex->pos.y == pos.y) 
        {
            vertex->refCount++;
            return vertex;
        }
    }

    static size_t vertexIndex = 0;

    struct MapVertex *vertex = calloc(1, sizeof *vertex);
    vertex->pos = pos;
    vertex->refCount = 1;
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

    state->gl.editorVertex.bufferMap[vertex->idx] = (struct VertexType){ .position = { pos.x, pos.y }, .color = { 1, 1, 1, 1 } };

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

    RemoveVertex(state, vertex, true);

    map->dirty = true;
}

struct MapVertex* EditGetVertex(struct EdState *state, struct Vertex pos)
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

struct MapVertex* EditGetClosestVertex(struct EdState *state, struct Vertex pos, float maxDist)
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
            line->refCount++;
            return line;
        }
    }

    const struct Vertex vert0 = v0->pos;
    const struct Vertex vert1 = v1->pos;
    int32_t normal = sign((vert0.x*vert1.y) - (vert0.y*vert1.x));

    static size_t lineIndex = 0;

    struct MapLine *line = calloc(1, sizeof *line);
    line->a = v0;
    line->b = v1;
    line->normal = normal;
    line->idx = lineIndex++;
    line->refCount = 1;
    line->prev = map->tailLine;

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

    vec2 l;
    glm_vec2_sub((vec2){ vert1.x, vert1.y }, (vec2){ vert0.x, vert0.y }, l);
    vec2 n = { -l[1], l[0] };
    glm_vec2_normalize(n);

    vec2 middle;
    glm_vec2_scale(l, 0.5f, middle);
    glm_vec2_add((vec2){ vert0.x, vert0.y }, middle, middle);

    vec2 middleNormal;
    glm_vec2_scale(n, 10.0f, middleNormal);
    glm_vec2_add(middle, middleNormal, middleNormal);

    state->gl.editorLine.bufferMap[line->idx * 4    ] = (struct VertexType){ .position = { vert0.x, vert0.y }, .color = { 1, 1, 1, 1 } };
    state->gl.editorLine.bufferMap[line->idx * 4 + 1] = (struct VertexType){ .position = { vert1.x, vert1.y }, .color = { 1, 1, 1, 1 } };
    state->gl.editorLine.bufferMap[line->idx * 4 + 2] = (struct VertexType){ .position = { middle[0], middle[1] }, .color = { 1, 1, 1, 1 } };
    state->gl.editorLine.bufferMap[line->idx * 4 + 3] = (struct VertexType){ .position = { middleNormal[0], middleNormal[1] }, .color = { 1, 1, 1, 1 } };

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

    RemoveLine(state, line);

    map->dirty = true;
}

struct MapLine* EditGetClosestLine(struct EdState *state, struct Vertex pos, float maxDist)
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
        RemoveLine(state, sector->outerLines[i]);
    }

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

struct MapSector* EditGetSector(struct EdState *state, struct Vertex pos)
{
    struct Map *map = &state->map;

    for(struct MapSector *sector = map->headSector; sector; sector = sector->next)
    {
        bool inside = false;
        for(size_t i = 0; i < sector->numOuterLines; ++i)
        {
            struct Vertex A = sector->outerLines[i]->a->pos;
            struct Vertex B = sector->outerLines[i]->b->pos;

            if ((pos.x == A.x && pos.y == A.y) || (pos.x == B.x && pos.y == B.y)) break;
            if (A.y == B.y && pos.y == A.y && between(pos.x, A.x, B.x)) break;

            if (between(pos.y, A.y, B.y)) 
            { // if P inside the vertical range
                // filter out "ray pass vertex" problem by treating the line a little lower
                if ((pos.y == A.y && B.y >= A.y) || (pos.y == B.y && A.y >= B.y)) continue;
                // calc cross product `PA X PB`, P lays on left side of AB if c > 0 
                float c = (A.x - pos.x) * (B.y - pos.y) - (B.x - pos.x) * (A.y - pos.y);
                if (c == 0) break;
                if ((A.y < B.y) == (c > 0)) inside = !inside;
            }
        }
        if(inside)
        {
            return sector;
        }
    }

    return NULL;
}

struct MapLine* EditApplyLines(struct EdState *state, struct Vertex *points, size_t num)
{
    return NULL;
}

struct MapSector* EditApplySector(struct EdState *state, struct Vertex *points, size_t num)
{
    struct Polygon *sourcePoly = malloc(sizeof *sourcePoly + num * sizeof *sourcePoly->vertices);
    sourcePoly->length = num;
    for(size_t i = 0; i < num; ++i)
    {
        sourcePoly->vertices[i][0] = points[i].x;
        sourcePoly->vertices[i][1] = points[i].y;
    }

    struct Map *map = &state->map;
    struct Polygon **sourceSimples;
    size_t numSimples = makeSimple(sourcePoly, &sourceSimples);
    struct MapSector *lastSectorAdded = NULL;

    LogDebug("Make simple: {d}", numSimples);

    for(size_t i = 0; i < numSimples; ++i)
    {
        struct Polygon *simple = sourceSimples[i];
        size_t numClipped = 0;
        struct Polygon *clippedSectors[1024];
        struct MapSector *sectorsToRemove[1024];
        for(struct MapSector *sector = map->headSector; sector; sector = sector->next)
        {
            struct Polygon *sectorPolygon = PolygonFromSector(&state->map, sector);
            bool res = intersects(sectorPolygon, simple);

            LogDebug("Sector {d} Clipped: {d}", sector->idx, res);

            if(res)
            {
                clippedSectors[numClipped] = sectorPolygon;
                sectorsToRemove[numClipped++] = sector;
            }
            else
                free(sectorPolygon);

            assert(numClipped < 1024);
        }

        if(numClipped == 0)
        {
            lastSectorAdded = AddPolygon(state, simple);
        }
        else
        {
            struct ClipResult2 res;
            clip2(simple, clippedSectors, numClipped, &res);

            for(size_t j = 0; j < numClipped; ++j)
                EditRemoveSector(state, sectorsToRemove[j]);

            for(size_t j = 0; j < res.numPolys; ++j)
            {
                struct Polygon *polygon = res.polygons[j];
                lastSectorAdded = AddPolygon(state, polygon);
            }
            freePolygons(res.polygons, res.numPolys);
            freePolygons(clippedSectors, numClipped);
        }
    }

    free(sourcePoly);
    freePolygons(sourceSimples, numSimples);
    free(sourceSimples);

    return lastSectorAdded;
}
