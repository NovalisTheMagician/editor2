#include "util.h"

#include <assert.h>

#include "../edit.h"
#include "map.h"
#include "map/query.h"
#include "remove.h"
#include "triangulate.h"

SplitResult SplitMapLine(Map *map, MapLine *line, MapVertex *vertex)
{
    LineData dataCopy = CopyLineData(line->data);

    MapVertex *va = GetVertex(map, line->a);
    MapVertex *vb = GetVertex(map, line->b);

    bool sharedSplitVertex = va == vertex || vb == vertex;
    vec2s splitPos = vertex->pos;

    RemoveLine(map, line);

    if(sharedSplitVertex)
        vertex = EditAddVertex(map, splitPos);

    MapLine *newA = EditAddLine(map, va, vertex, dataCopy);
    MapLine *newB = EditAddLine(map, vertex, vb, dataCopy);

    FreeLineData(dataCopy);

    return (SplitResult){ .left = newA, .right = newB };
}

SplitResult SplitMapLine2(Map *map, MapLine *line, MapVertex *vertexA, MapVertex *vertexB)
{
    LineData dataCopy = CopyLineData(line->data);

    MapVertex *va = GetVertex(map, line->a);
    MapVertex *vb = GetVertex(map, line->b);

    bool sharedSplitVertexA = va == vertexA || vb == vertexA;
    bool sharedSplitVertexB = va == vertexB || vb == vertexB;
    vec2s splitPosA = vertexA->pos;
    vec2s splitPosB = vertexB->pos;

    RemoveLine(map, line);

    if(sharedSplitVertexA)
        vertexA = EditAddVertex(map, splitPosA);

    if(sharedSplitVertexB)
        vertexB = EditAddVertex(map, splitPosB);

    MapLine *newStart = EditAddLine(map, va, vertexA, dataCopy);
    MapLine *newMiddle = EditAddLine(map, vertexA, vertexB, dataCopy);
    MapLine *newEnd = EditAddLine(map, vertexB, vb, dataCopy);

    FreeLineData(dataCopy);

    return (SplitResult){ .left = newStart, .middle = newMiddle, .right = newEnd };
}

struct Polygon* PolygonFromMapLines(Map *map, size_t numLines, MapLine *lines[static numLines])
{
    assert(numLines >= 3);
    struct Polygon *polygon = calloc(1, sizeof *polygon + numLines * sizeof *polygon->vertices);
    polygon->length = numLines;

    size_t vertex = lines[0]->a, nextVertex = lines[0]->b;
    polygon->vertices[0][0] = GetVertex(map, vertex)->pos.x;
    polygon->vertices[0][1] = GetVertex(map, vertex)->pos.y;
    for(size_t i = 1; i < numLines; ++i)
    {
        MapLine *mapLine = lines[i];
        bool front = mapLine->a == nextVertex;
        vertex = front ? mapLine->a : mapLine->b;
        nextVertex = front ? mapLine->b : mapLine->a;

        polygon->vertices[i][0] = GetVertex(map, vertex)->pos.x;
        polygon->vertices[i][1] = GetVertex(map, vertex)->pos.y;
    }

    return polygon;
}

struct Polygon* PolygonFromVertices(size_t numVertices, EditorVertexType vertices[static numVertices])
{
    struct Polygon *polygon = calloc(1, sizeof *polygon + numVertices * sizeof *polygon->vertices);
    polygon->length = numVertices;
    for(size_t i = 0; i < numVertices; ++i)
    {
        polygon->vertices[i][0] = vertices[i].position.x;
        polygon->vertices[i][1] = vertices[i].position.y;
    }
    return polygon;
}

struct Polygon* PolygonFromVectors(size_t numVectors, vec2s vectors[static numVectors])
{
    struct Polygon *polygon = calloc(1, sizeof *polygon + numVectors * sizeof *polygon->vertices);
    polygon->length = numVectors;
    for(size_t i = 0; i < numVectors; ++i)
    {
        polygon->vertices[i][0] = vectors[i].x;
        polygon->vertices[i][1] = vectors[i].y;
    }
    return polygon;
}

bool IsLineFront(MapVertex *v1, MapLine *line)
{
    assert(v1->idx == line->a || v1->idx == line->b);
    return v1->idx == line->a;
}

MapLine* GetMapLine(Map *map, line_t line)
{
    for(size_t i = 0; i < map->lineList.count; ++i)
    {
        MapLine *mline = &map->lineList.items[i];
        MapVertex *a = GetVertex(map, mline->a);
        MapVertex *b = GetVertex(map, mline->b);
        if((glms_vec2_eqv_eps(a->pos, line.a) && glms_vec2_eqv_eps(b->pos, line.b)) ||
           (glms_vec2_eqv_eps(b->pos, line.a) && glms_vec2_eqv_eps(a->pos, line.b)))
           return mline;
    }
    return NULL;
}
