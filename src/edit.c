#include "edit.h"


#include <assert.h>
#include <tgmath.h>
#include <string.h>

#include "triangulate.h"

#include "geometry.h"
#include "map.h"
#include "map/remove.h"
#include "map/util.h"
#include "map/insert.h"
#include "map/create.h"
#include "memory.h" // IWYU pragma: keep

void ScreenToEditorSpace(const EdState *state, int32_t *x, int32_t *y)
{
    const float z = state->data.zoomLevel;
    *x = (int32_t)((*x + state->data.viewPosition.x) / z);
    *y = (int32_t)((*y + state->data.viewPosition.y) / z);
}

void ScreenToEditorSpacef(const EdState *state, float *x, float *y)
{
    const float z = state->data.zoomLevel;
    *x = (*x + state->data.viewPosition.x) / z;
    *y = (*y + state->data.viewPosition.y) / z;
}

void EditorToScreenSpace(const EdState *state, int32_t *x, int32_t *y)
{
    const float z = state->data.zoomLevel;
    *x = (int32_t)((*x - state->data.viewPosition.x) * z);
    *y = (int32_t)((*y - state->data.viewPosition.y) * z);
}

void ScreenToEditorSpaceGrid(const EdState *state, int32_t *x, int32_t *y)
{
    ScreenToEditorSpace(state, x, y);
    const int offset = state->data.gridSize / 2;
    int32_t xt = *x, yt = *y;
    xt += xt < 0 ? -offset : offset;
    *x = xt / state->data.gridSize * state->data.gridSize;
    yt += yt < 0 ? -offset : offset;
    *y = yt / state->data.gridSize * state->data.gridSize;
}

void EditCopy(EdState *state)
{
    LogDebug("Copy!!\n");
}

void EditPaste(EdState *state)
{
    LogDebug("Paste!!\n");
}

void EditCut(EdState *state)
{
    LogDebug("Cut!!\n");
}

MapVertex* EditAddVertex(Map *map, vec2s pos)
{
    CreateResult result = CreateVertex(map, pos);
    if(!result.created) return result.mapElement;
    MapVertex *vertex = result.mapElement;

    return vertex;
}

void EditRemoveVertices(Map *map, size_t num, MapVertex *vertices[static num])
{
    MapLine *potentialLines[4096] = { 0 };
    size_t numPotentialLines = 0;

    for(size_t i = 0; i < num; ++i)
    {
        MapVertex *vertex = vertices[i];
        for(size_t j = 0; j < vertex->numAttachedLines; ++j)
        {
            bool lineIsInSet = false;
            MapLine *attLine = vertex->attachedLines[j];
            for(size_t k = 0; k < numPotentialLines; ++k)
            {
                lineIsInSet |= potentialLines[k] == attLine;
                if(lineIsInSet) break;
            }

            if(!lineIsInSet) potentialLines[numPotentialLines++] = attLine;
        }
        RemoveVertex(map, vertex);
    }

    MapSector *potentialSectors[4096] = { 0 };
    size_t numPotentialSectors = 0;

    for(size_t i = 0; i < numPotentialLines; ++i)
    {
        MapLine *line = potentialLines[i];
        if(!line->a || !line->b)
        {
            bool isFrontInSet = line->frontSector == NULL;
            bool isBackInSet = line->backSector == NULL;
            for(size_t s = 0; s < numPotentialSectors; ++s)
            {
                if(!isFrontInSet) isFrontInSet = potentialSectors[s] == line->frontSector;
                if(!isBackInSet) isBackInSet = potentialSectors[s] == line->backSector;
                if(isFrontInSet && isBackInSet) break;
            }
            if(!isFrontInSet) potentialSectors[numPotentialSectors++] = line->frontSector;
            if(!isBackInSet) potentialSectors[numPotentialSectors++] = line->backSector;

            RemoveLine(map, line);
        }
    }

    for(size_t i = 0; i < numPotentialSectors; ++i)
    {
        MapSector *sector = potentialSectors[i];
        RemoveSector(map, sector);
    }

    map->dirty = true;
}

MapVertex* EditGetVertex(Map *map, vec2s pos)
{
    for(MapVertex *vertex = map->headVertex; vertex; vertex = vertex->next)
    {
        if(vertex->pos.x == pos.x && vertex->pos.y == pos.y)
        {
            return vertex;
        }
    }
    return NULL;
}

MapVertex* EditGetClosestVertex(Map *map, vec2s pos, float maxDist)
{
    MapVertex *closestVertex = NULL;
    float closestDist = FLT_MAX;
    for(MapVertex *vertex = map->headVertex; vertex; vertex = vertex->next)
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

MapLine* EditAddLine(Map *map, MapVertex *v0, MapVertex *v1, LineData data)
{
    CreateResult result = CreateLine(map, v0, v1, data);
    if(!result.created) return result.mapElement;
    MapLine *line = result.mapElement;

    vec2s vert0 = line->a->pos;
    vec2s vert1 = line->b->pos;

    vec2s l = glms_vec2_sub(vert1, vert0);
    vec2s n = {{ -l.y, l.x }};
    n = glms_vec2_normalize(n);

    vec2s middle = glms_vec2_scale(l, 0.5f);
    middle = glms_vec2_add(vert0, middle);

    vec2s middleNormal = glms_vec2_scale(n, 10.0f);
    middleNormal = glms_vec2_add(middle, middleNormal);

    return line;
}

void EditRemoveLines(Map *map, size_t num, MapLine *lines[static num])
{
    MapVertex *potentialVertices[4096] = { 0 };
    MapSector *potentialSectors[4096] = { 0 };
    size_t numPotentialVertices = 0;
    size_t numPotentialSectors = 0;

    for(size_t i = 0; i < num; ++i)
    {
        MapLine *line = lines[i];

        bool isAInSet = false;
        bool isBInSet = false;
        for(size_t v = 0; v < numPotentialVertices; ++v)
        {
            if(!isAInSet) isAInSet = potentialVertices[v] == line->a;
            if(!isBInSet) isBInSet = potentialVertices[v] == line->b;
            if(isAInSet && isBInSet) break;
        }
        if(!isAInSet) potentialVertices[numPotentialVertices++] = line->a;
        if(!isBInSet) potentialVertices[numPotentialVertices++] = line->b;

        bool isFrontInSet = line->frontSector == NULL;
        bool isBackInSet = line->backSector == NULL;
        for(size_t s = 0; s < numPotentialSectors; ++s)
        {
            if(!isFrontInSet) isFrontInSet = potentialSectors[s] == line->frontSector;
            if(!isBackInSet) isBackInSet = potentialSectors[s] == line->backSector;
            if(isFrontInSet && isBackInSet) break;
        }
        if(!isFrontInSet) potentialSectors[numPotentialSectors++] = line->frontSector;
        if(!isBackInSet) potentialSectors[numPotentialSectors++] = line->backSector;

        RemoveLine(map, line);
    }

    for(size_t i = 0; i < numPotentialVertices; ++i)
    {
        MapVertex *vertex = potentialVertices[i];
        if(vertex->numAttachedLines == 0) RemoveVertex(map, vertex);
    }

    for(size_t i = 0; i < numPotentialSectors; ++i)
    {
        MapSector *sector = potentialSectors[i];
        RemoveSector(map, sector);
    }

    map->dirty = true;
}

MapLine* EditGetClosestLine(Map *map, vec2s pos, float maxDist)
{
    MapLine *closestLine = NULL;
    float closestDist = FLT_MAX;
    for(MapLine *line = map->headLine; line; line = line->next)
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

static bool includes(size_t num, void *elements[static num], void *element)
{
    for(size_t i = 0; i < num; ++i)
    {
        if(elements[i] == element)
            return true;
    }
    return false;
}

static void setLineSector(size_t numLines, MapLine *lines[static numLines], bool firstFront, MapSector *sector)
{
    MapVertex *nextVertex = firstFront ? lines[0]->b : lines[0]->a;
    if(firstFront)
    {
        lines[0]->frontSector = sector;
        if(lines[0]->backSector && !includes(sector->numContains, (void**)sector->contains, lines[0]->backSector))
        {
            size_t idx = sector->numContains++;
            sector->contains = realloc(sector->contains, sector->numContains * sizeof *sector->contains);
            sector->contains[idx] = lines[0]->backSector;
        }
    }
    else
    {
        lines[0]->backSector = sector;
        if(lines[0]->frontSector && !includes(sector->numContains, (void**)sector->contains, lines[0]->frontSector))
        {
            size_t idx = sector->numContains++;
            sector->contains = realloc(sector->contains, sector->numContains * sizeof *sector->contains);
            sector->contains[idx] = lines[0]->frontSector;
        }
    }

    for(size_t i = 1; i < numLines; ++i)
    {
        MapLine *line = lines[i];
        if(line->a == nextVertex)
        {
            line->frontSector = sector;
            if(line->backSector && !includes(sector->numContains, (void**)sector->contains, line->backSector))
            {
                size_t idx = sector->numContains++;
                sector->contains = realloc(sector->contains, sector->numContains * sizeof *sector->contains);
                sector->contains[idx] = line->backSector;
            }
            nextVertex = line->b;
        }
        else
        {
            line->backSector = sector;
            if(line->frontSector && !includes(sector->numContains, (void**)sector->contains, line->frontSector))
            {
                size_t idx = sector->numContains++;
                sector->contains = realloc(sector->contains, sector->numContains * sizeof *sector->contains);
                sector->contains[idx] = line->frontSector;
            }
            nextVertex = line->a;
        }
    }
}

MapSector* EditAddSector(Map *map, size_t numLines, MapLine *lines[static numLines], size_t numInnerLines, size_t numInnerLinesNum[static numInnerLines], MapLine ***innerLines, SectorData data)
{
    CreateResult result = CreateSector(map, numLines, lines, data);
    if(!result.created) return result.mapElement;
    MapSector *sector = result.mapElement;

    struct Polygon *polygon = PolygonFromMapLines(numLines, lines);
    orientation_t orientation = LineLoopOrientation(polygon->length, (vec2s*)polygon->vertices);
    setLineSector(numLines, lines, orientation == CW_ORIENT, sector);

    struct Polygon **innerPolygons = calloc(numInnerLines, sizeof *innerPolygons);
    for(size_t i = 0; i < numInnerLines; ++i)
    {
        innerPolygons[i] = PolygonFromMapLines(numInnerLinesNum[i], innerLines[i]);
        orientation = LineLoopOrientation(innerPolygons[i]->length, (vec2s*)innerPolygons[i]->vertices);
        setLineSector(numInnerLinesNum[i], innerLines[i], orientation == CCW_ORIENT, sector);
    }

    TriangleData *td = &sector->edData;

    unsigned int *indices = NULL;
    size_t numIndices = triangulate(polygon, innerPolygons, numInnerLines, &indices);

    td->indices = malloc(numIndices * sizeof *indices);
    memcpy(td->indices, indices, numIndices * sizeof *indices);
    td->numIndices = numIndices;
    free(indices);

    td->numVertices = polygon->length;
    for(size_t i = 0; i < numInnerLines; ++i)
        td->numVertices += innerPolygons[i]->length;
    td->vertices = calloc(td->numVertices, sizeof *td->vertices);
    memcpy(td->vertices, polygon->vertices, polygon->length * sizeof *polygon->vertices);
    size_t offset = polygon->length;
    for(size_t i = 0; i < numInnerLines; ++i)
    {
        memcpy(td->vertices + offset, innerPolygons[i]->vertices, innerPolygons[i]->length * sizeof *(innerPolygons[i]->vertices));
        offset += innerPolygons[i]->length;
    }

    free(polygon);
    for(size_t i = 0; i < numInnerLines; ++i)
        free(innerPolygons[i]);
    free(innerPolygons);

    sector->bb = BoundingBoxFromVertices(td->numVertices, td->vertices);

    return sector;
}

void EditRemoveSectors(Map *map, size_t num, MapSector *sectors[static num])
{
    for(size_t i = 0; i < num; ++i)
    {
        MapSector *sector = sectors[i];
        RemoveSector(map, sector);
    }

    map->dirty = true;
}

MapSector* EditGetSector(Map *map, vec2s pos)
{
    for(MapSector *sector = map->headSector; sector; sector = sector->next)
    {
        bool isIn = PointInSector2(sector, pos);
        if(isIn) return sector;
    }
    return NULL;
}

bool EditApplyLines(EdState *state, size_t num, vec2s points[static num])
{
    Map *map = &state->map;
    return InsertLinesIntoMap(map, num, points, false);
}

bool EditApplySector(EdState *state, size_t num, vec2s points[static num])
{
    Map *map = &state->map;
    return InsertLinesIntoMap(map, num, points, true);
}
