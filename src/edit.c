#include "edit.h"

#include <assert.h>
#include <math.h>
#include <string.h>

#include "arena.h"

#include "map/query.h"
#include "triangulate.h"

#include "geometry.h"
#include "map.h"
#include "map/remove.h"
#include "map/util.h"
#include "map/insert.h"
#include "map/create.h"

void ScreenToEditorSpace(const EdState *state, real_t *x, real_t *y)
{
    const float z = state->data.zoomLevel;
    *x = (*x + state->data.viewPosition.x) / z;
    *y = (*y + state->data.viewPosition.y) / z;
}

void EditorToScreenSpace(const EdState *state, real_t *x, real_t *y)
{
    const float z = state->data.zoomLevel;
    *x = ((*x - state->data.viewPosition.x) * z);
    *y = ((*y - state->data.viewPosition.y) * z);
}

void ScreenToEditorSpaceGrid(const EdState *state, int gridsize, real_t *x, real_t *y)
{
    ScreenToEditorSpace(state, x, y);
    const int offset = gridsize / 2;
    real_t xt = (int)(*x), yt = (int)(*y);
    xt += xt < 0 ? -offset : offset;
    *x = (int)xt / gridsize * gridsize;
    yt += yt < 0 ? -offset : offset;
    *y = (int)yt / gridsize * gridsize;
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

MapVertex* EditAddVertex(Map *map, FVec2 pos)
{
    MapVertex *existing = FindClosestVertex(map, pos, STITCHING_DIST);
    if(existing) return existing;
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

    MapSector *deletedSectors[4096] = { 0 };
    size_t numDeletedSectors = 0;

    for(size_t i = 0; i < numPotentialLines; ++i)
    {
        MapLine *line = potentialLines[i];
        if(!line->a || !line->b)
        {
            MapSector *frontSector = line->frontSector;
            MapSector *backSector = line->backSector;
            bool canDeleteFront = frontSector != NULL, canDeleteBack = backSector != NULL;
            for(size_t s = 0; s < numDeletedSectors; ++s)
            {
                if(!canDeleteFront && !canDeleteBack)
                    break;
                if(frontSector == deletedSectors[s])
                    canDeleteFront = false;
                if(backSector == deletedSectors[s])
                    canDeleteBack = false;
            }
            if(canDeleteFront)
            {
                RemoveSector(map, frontSector);
                deletedSectors[numDeletedSectors++] = frontSector;
            }
            if(canDeleteBack)
            {
                RemoveSector(map, backSector);
                deletedSectors[numDeletedSectors++] = backSector;
            }

            RemoveLine(map, line);
        }
    }

    map->dirty = true;
}

MapVertex* EditGetVertex(Map *map, FVec2 pos)
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

MapVertex* EditGetClosestVertex(Map *map, FVec2 pos, fixed_t maxDist)
{
    return FindClosestVertex(map, pos, maxDist);
}

MapLine* EditAddLine(Map *map, MapVertex *v0, MapVertex *v1, LineData data)
{
    CreateResult result = CreateLine(map, v0, v1, data);
    if(!result.created) return result.mapElement;
    MapLine *line = result.mapElement;

    return line;
}

void EditRemoveLines(Map *map, size_t num, MapLine *lines[static num])
{
    MapVertex *potentialVertices[4096] = { 0 };
    MapSector *deletedSectors[4096] = { 0 };
    size_t numPotentialVertices = 0;
    size_t numDeletedSectors = 0;

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

        MapSector *frontSector = line->frontSector;
        MapSector *backSector = line->backSector;
        if(frontSector == backSector)
            backSector = NULL;
        bool canDeleteFront = frontSector != NULL, canDeleteBack = backSector != NULL;

        for(size_t s = 0; s < numDeletedSectors; ++s)
        {
            if(!canDeleteFront && !canDeleteBack)
                break;
            if(frontSector == deletedSectors[s])
                canDeleteFront = false;
            if(backSector == deletedSectors[s])
                canDeleteBack = false;
        }
        if(canDeleteFront)
        {
            RemoveSector(map, frontSector);
            deletedSectors[numDeletedSectors++] = frontSector;
        }
        if(canDeleteBack)
        {
            RemoveSector(map, backSector);
            deletedSectors[numDeletedSectors++] = backSector;
        }

        RemoveLine(map, line);
    }

    for(size_t i = 0; i < numPotentialVertices; ++i)
    {
        MapVertex *vertex = potentialVertices[i];
        if(vertex->numAttachedLines == 0) RemoveVertex(map, vertex);
    }

    map->dirty = true;
}

MapLine* EditGetClosestLine(Map *map, FVec2 pos, fixed_t maxDist)
{
    MapLine *closestLine = NULL;
    real_t closestDist = FLT_MAX;
    real_t md = fixed_to_real(maxDist);
    for(MapLine *line = map->headLine; line; line = line->next)
    {
        real_t dist = MinDistToLine(line->a->pos, line->b->pos, pos);
        if(dist <= md && dist < closestDist)
        {
            closestDist = dist;
            closestLine = line;
        }
    }
    return closestLine;
}

static void setLineSector(size_t numLines, MapLine *lines[static numLines], MapSector *sector, bool otherSide)
{
    MapVertex *nextVertex, *vertex;
    GetLoopStart(lines[0], lines[1], &vertex, &nextVertex);

    bool front0 = vertex == lines[0]->a;
    if(front0 != otherSide)
        lines[0]->frontSector = sector;
    else
        lines[0]->backSector = sector;

    for(size_t i = 1; i < numLines; ++i)
    {
        MapLine *line = lines[i];
        bool front = line->a == nextVertex;
        if(front != otherSide)
            line->frontSector = sector;
        else
            line->backSector = sector;

        nextVertex = front ? line->b : line->a;
    }
}

static Arena arena = { 0 };

static struct Polygon* constructPolygon(Arena *arena, size_t numLines, MapLine *lines[static numLines])
{
    struct Polygon *polygon = arena_alloc(arena, sizeof *polygon + numLines * sizeof *polygon->vertices);

    MapVertex *vertex, *nextVertex;
    GetLoopStart(lines[0], lines[1], &vertex, &nextVertex);

    polygon->vertices[0][0] = fixed_to_real(vertex->pos.x);
    polygon->vertices[0][1] = fixed_to_real(vertex->pos.y);
    for(size_t i = 1; i < numLines; ++i)
    {
        MapLine *mapLine = lines[i];
        bool front = mapLine->a == nextVertex;
        vertex = front ? mapLine->a : mapLine->b;
        nextVertex = front ? mapLine->b : mapLine->a;

        polygon->vertices[i][0] = fixed_to_real(vertex->pos.x);
        polygon->vertices[i][1] = fixed_to_real(vertex->pos.y);
    }
    polygon->length = numLines;
    return polygon;
}

MapSector* EditAddSector(Map *map, size_t numLines, MapLine *lines[static numLines], size_t numInnerLines, size_t numInnerLinesNum[static numInnerLines], MapLine ***innerLines, bool innerOtherSide[static numInnerLines], SectorData data)
{
    CreateResult result = CreateSector(map, numLines, lines, data);
    if(!result.created) return result.mapElement;
    MapSector *sector = result.mapElement;

    arena_reset(&arena);

    sector->innerLines = malloc(numInnerLines * sizeof *sector->innerLines);
    for(size_t i = 0; i < numInnerLines; ++i)
    {
        size_t n = numInnerLinesNum[i];
        if(n == 0)
            continue;
        size_t totalSize = n * sizeof *sector->innerLines[i];
        sector->innerLines[i] = malloc(totalSize);
        memcpy(sector->innerLines[i], innerLines[i], totalSize);
    }
    sector->numInnerLines = numInnerLines;
    sector->numInnerLinesNum = malloc(numInnerLines * sizeof *sector->numInnerLinesNum);
    memcpy(sector->numInnerLinesNum, numInnerLinesNum, numInnerLines * sizeof *sector->numInnerLinesNum);

    struct Polygon *polygon = constructPolygon(&arena, numLines, lines);
    orientation_t orientation = LineLoopOrientationReal(polygon->length, (Vec2*)polygon->vertices);
    setLineSector(numLines, lines, sector, false);

    struct Polygon **innerPolygons = arena_alloc(&arena, numInnerLines * sizeof *innerPolygons);
    for(size_t i = 0; i < numInnerLines; ++i)
    {
        if(numInnerLinesNum[i] == 0)
            continue;
        innerPolygons[i] = constructPolygon(&arena, numInnerLinesNum[i], innerLines[i]);
        orientation = LineLoopOrientationReal(innerPolygons[i]->length, (Vec2*)innerPolygons[i]->vertices);
        setLineSector(numInnerLinesNum[i], innerLines[i], sector, innerOtherSide[i]);
    }

    TriangleData *td = &sector->edData;

    uint32_t *indices = NULL;
    size_t numIndices = triangulate(polygon, innerPolygons, numInnerLines, &indices);

    td->indices = malloc(numIndices * sizeof *indices);
    memcpy(td->indices, indices, numIndices * sizeof *indices);
    td->numIndices = numIndices;
    free(indices);

    td->numVertices = polygon->length;
    for(size_t i = 0; i < numInnerLines; ++i)
        td->numVertices += innerPolygons[i]->length;
    td->vertices = calloc(td->numVertices, sizeof *td->vertices);
    for(size_t i = 0; i < polygon->length; ++i)
    {
        td->vertices[i].x = (real_t)polygon->vertices[i][0];
        td->vertices[i].y = (real_t)polygon->vertices[i][1];
    }
    size_t offset = polygon->length;
    for(size_t i = 0; i < numInnerLines; ++i)
    {
        for(size_t j = 0; j < innerPolygons[i]->length; ++j)
        {
            td->vertices[offset+j].x = (real_t)innerPolygons[i]->vertices[j][0];
            td->vertices[offset+j].y = (real_t)innerPolygons[i]->vertices[j][1];
        }
        offset += innerPolygons[i]->length;
    }

    sector->bb = BoundingBoxFromVerticesReal(td->numVertices, td->vertices);

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

MapSector* EditGetSector(Map *map, FVec2 pos)
{
    for(MapSector *sector = map->headSector; sector; sector = sector->next)
    {
        bool isIn = PointInSector2(sector, pos);
        if(isIn) return sector;
    }
    return NULL;
}

bool EditApplyLines(EdState *state, size_t num, FVec2 points[static num])
{
    Map *map = &state->map;
    return InsertLinesIntoMap(map, num, points, false);
}

bool EditApplySector(EdState *state, size_t num, FVec2 points[static num])
{
    Map *map = &state->map;
    return InsertLinesIntoMap(map, num, points, true);
}
