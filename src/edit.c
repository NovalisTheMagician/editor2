#include "edit.h"
#include "map.h"

#include <assert.h>

#include <triangulate.h>

static int32_t sign(int32_t x) {
    return (x > 0) - (x < 0);
}

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
    LogInfo("Copy!!\n");
}

void EditPaste(struct EdState *state)
{
    LogInfo("Paste!!\n");
}

void EditCut(struct EdState *state)
{
    LogInfo("Cut!!\n");
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

    struct MapVertex *vertex = calloc(1, sizeof *vertex);
    vertex->pos = pos;
    vertex->refCount = 1;
    vertex->idx = map->numVertices++;
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
    struct Map *map = &state->map;
    assert(vertex);
    // remove lines affected by vertex

    map->dirty = true;
}

static void RemoveVertex(struct EdState *state, struct MapVertex *vertex)
{
    struct Map *map = &state->map;
    assert(vertex);
    assert(vertex->refCount > 0);

    vertex->refCount--;
    if(vertex->refCount == 0)
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

        for(struct MapVertex *v = next; v; v = v->next)
        {
            v->idx--;
        }

        memmove(state->gl.editorVertex.bufferMap + vertex->idx, state->gl.editorVertex.bufferMap + vertex->idx + 1, (map->numVertices - (vertex->idx+1)) * sizeof *state->gl.editorVertex.bufferMap);

        free(vertex);

        map->numVertices--;
        map->dirty = true;
    }
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

    struct MapLine *line = calloc(1, sizeof *line);
    line->a = v0;
    line->b = v1;
    line->normal = normal;
    line->idx = map->numLines++;
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
    struct Map *map = &state->map;
    map->dirty = true;
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

        for(struct MapLine *l = next; l; l = l->next)
        {
            l->idx--;
        }

        RemoveVertex(state, line->a);
        RemoveVertex(state, line->b);

        memmove(state->gl.editorLine.bufferMap + line->idx * 4, state->gl.editorLine.bufferMap + (line->idx + 1) * 4, (map->numLines - (line->idx+1)) * 4 * sizeof *state->gl.editorLine.bufferMap);

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

struct MapLine* EditGetLine(struct EdState *state, struct Vertex pos)
{
    return NULL;
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

/*
struct MapSector* EditAddSector(struct EdState *state, struct MapLine *lines, size_t numLines)
{
    struct Map *map = &state->map;

    size_t idx = map->numSectors++;
    struct Sector *s = &map->sectors[idx];
    s->lines = calloc(numLines, sizeof *s->lines);
    s->numLines = numLines;
    for(size_t i = 0; i < numLines; ++i)
        s->lines[i] = lineIndices[i];

    if(map->numSectors == map->numAllocSectors)
    {
        IncreaseBufferSize((void**)&map->sectors, &map->numAllocSectors, sizeof *map->sectors);
        state->sectorToPolygon = realloc(state->sectorToPolygon, map->numAllocSectors * sizeof *state->sectorToPolygon);
    }

    size_t baseVertexIndex = state->gl.editorSector.highestVertIndex;
    size_t baseIndexIndex = state->gl.editorSector.highestIndIndex;

    size_t index = baseVertexIndex;
    struct Line lastLine;
    size_t lastIndex;
    struct Polygon *outerPolygon = calloc(1, sizeof *outerPolygon + numLines * sizeof *outerPolygon->vertices);
    outerPolygon->length = numLines;
    for(size_t i = 0; i < numLines; ++i)
    {
        struct Line line = map->lines[lineIndices[i]];
        size_t vertIdx;
        if(i == 0)
            vertIdx = line.a;
        else 
        {
            size_t lastEnd = lastLine.a == lastIndex ? lastLine.b : lastLine.a;
            vertIdx = line.a == lastEnd ? line.a : line.b;
        }
        lastLine = line;
        lastIndex = vertIdx;

        int32_t x = map->vertices[vertIdx].x;
        int32_t y = map->vertices[vertIdx].y;
        state->gl.editorSector.bufferMap[index++] = (struct SectorVertexType){ .position = { x, y }, .color = { 1, 1, 1, 1 }, .texCoord = { 0, 0 } };
        outerPolygon->vertices[i][0] = x;
        outerPolygon->vertices[i][1] = y;
    }
    unsigned int *indices = NULL;
    unsigned long numIndices = triangulate(outerPolygon, NULL, 0, &indices);

    index = baseIndexIndex;
    for(size_t i = 0; i < numIndices; ++i)
        state->gl.editorSector.indexMap[index++] = indices[i] + baseVertexIndex;

    free(outerPolygon);
    free(indices);

    state->sectorToPolygon[idx] = (__typeof__(*state->sectorToPolygon)){ .indexStart = baseIndexIndex, .indexLength = numIndices, .vertexStart = baseVertexIndex, .vertexLength = numLines };

    state->gl.editorSector.highestVertIndex += numLines;
    state->gl.editorSector.highestIndIndex += numIndices;

    map->dirty = true;
    return idx;
}
*/

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

    __typeof__(*state->sectorToPolygon) secPoly = state->sectorToPolygon[sector->idx];
    for(size_t i = sector->idx+1; i < map->numSectors; ++i)
    {
        state->sectorToPolygon[i].indexStart -= secPoly.indexLength;
        state->sectorToPolygon[i].vertexStart -= secPoly.vertexLength;
    }

    memmove(state->sectorToPolygon + sector->idx, state->sectorToPolygon + sector->idx + 1, (map->numSectors - (sector->idx+1)) * sizeof *state->sectorToPolygon);
    memmove(state->gl.editorSector.bufferMap + secPoly.vertexStart, state->gl.editorSector.bufferMap + secPoly.vertexStart + secPoly.vertexLength, (state->gl.editorSector.highestVertIndex - (secPoly.vertexStart + secPoly.vertexLength - 1)) * sizeof *state->gl.editorSector.bufferMap);
    memmove(state->gl.editorSector.indexMap + secPoly.indexStart, state->gl.editorSector.indexMap + secPoly.indexStart + secPoly.indexLength, (state->gl.editorSector.highestIndIndex - (secPoly.indexStart + secPoly.indexLength - 1)) * sizeof *state->gl.editorSector.indexMap);

    state->gl.editorSector.highestIndIndex -= secPoly.indexLength;
    state->gl.editorSector.highestVertIndex -= secPoly.vertexLength;

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

#define between(p, a, b) (((p) >= (a) && (p) <= (b)) || ((p) <= (a) && (p) >= (b)))

struct MapSector* EditGetSector(struct EdState *state, struct Vertex pos)
{
    struct Map *map = &state->map;

    for(struct MapSector *sector = map->headSector; sector; sector = sector->next)
    {
        bool inside = false;
        for(size_t i = 0; i < sector->numOuterLines; ++i)
        {
            struct Vertex A = sector->outerLines[i]->a->pos; // map->vertices[map->lines[sector->lines[i]].a];
            struct Vertex B = sector->outerLines[i]->b->pos; // map->vertices[map->lines[sector->lines[i]].b];

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

static struct MapSector* AddPolygon(struct EdState *state, struct Polygon *polygon)
{
    struct Map *map = &state->map;
    assert(polygon);

    struct MapSector *sector = calloc(1, sizeof *sector);
    sector->numOuterLines = polygon->length;
    sector->outerLines = malloc(sector->numOuterLines * sizeof *sector->outerLines);
    sector->idx = map->numSectors++;
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

    state->sectorToPolygon[sector->idx] = (__typeof__(*state->sectorToPolygon)){ .indexStart = baseIndexIndex, .indexLength = numIndices, .vertexStart = baseVertexIndex, .vertexLength = polygon->length };

    if(state->sectorToPolygonAlloc == map->numSectors)
    {
        state->sectorToPolygonAlloc *= 2;
        state->sectorToPolygon = realloc(state->sectorToPolygon, state->sectorToPolygonAlloc * sizeof *state->sectorToPolygon);
    }

    state->gl.editorSector.highestVertIndex += polygon->length;
    state->gl.editorSector.highestIndIndex += numIndices;

    map->dirty = true;
    return sector;
}

static struct Polygon* PolygonFromSector(struct Map *map, struct MapSector *sector)
{
    struct Polygon *polygon = calloc(1, sizeof *polygon + sector->numOuterLines * sizeof *polygon->vertices);
    polygon->length = sector->numOuterLines;
    for(size_t i = 0; i < sector->numOuterLines; ++i)
    {
        struct Vertex v = sector->outerLines[i]->a->pos;
        polygon->vertices[i][0] = v.x;
        polygon->vertices[i][1] = v.y;
    }
    return polygon;
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

    LogInfo("Make simple: {d}", numSimples);

    if(state->map.numSectors == 0)
    {
        for(size_t i = 0; i < numSimples; ++i)
        {
            lastSectorAdded = AddPolygon(state, sourceSimples[i]);
        }
    }
    else
    {
        for(size_t i = 0; i < numSimples; ++i)
        {
            struct Polygon *simple = sourceSimples[i];
            size_t numClipped = 0;
            struct 
            {
                struct ClipResult res;
                struct MapSector *sector;
            } results[1024];
            for(struct MapSector *sector = map->headSector; sector; sector = sector->next)
            {
                struct Polygon *sectorPolygon = PolygonFromSector(&state->map, sector);
                struct ClipResult res = clip(sectorPolygon, simple);

                LogInfo("A Clipped: {d}", res.numAClipped);
                LogInfo("B Clipped: {d}", res.numBClipped);
                LogInfo("C Clipped: {d}", res.numNewPolygons);

                bool clipped = res.numAClipped > 0 || res.numBClipped > 0 || res.numNewPolygons > 0;
                numClipped += clipped;
                
                free(sectorPolygon);
                if(clipped)
                {
                    results[numClipped-1].res = res;
                    results[numClipped-1].sector = sector;
                }
                else
                {
                    freeClipResults(res);
                }
            }

            if(numClipped == 0)
            {
                lastSectorAdded = AddPolygon(state, simple);
            }
            else
            {
                for(size_t j = 0; j < numClipped; ++j)
                {
                    struct ClipResult res = results[j].res;
                    struct MapSector *sector = results[j].sector;

                    if(res.numAClipped > 0)
                    {
                        EditRemoveSector(state, sector);
                    }

                    for(size_t secPolyIdx = 0; secPolyIdx < res.numAClipped; ++secPolyIdx)
                    {
                        lastSectorAdded = AddPolygon(state, res.aClipped[secPolyIdx]);
                    }

                    for(size_t editPolyIdx = 0; editPolyIdx < res.numBClipped; ++editPolyIdx)
                    {
                        lastSectorAdded = AddPolygon(state, res.bClipped[editPolyIdx]);
                    }

                    for(size_t newPolyIdx = 0; newPolyIdx < res.numNewPolygons; ++newPolyIdx)
                    {
                        lastSectorAdded = AddPolygon(state, res.newPolygons[newPolyIdx]);
                    }

                    freeClipResults(res);
                }
            }
        }
    }

    free(sourcePoly);
    freePolygons(sourceSimples, numSimples);
    free(sourceSimples);

    return lastSectorAdded;
}
