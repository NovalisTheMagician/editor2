#include "util.h"

#include <assert.h>

#include "../edit.h"
#include "map.h"
#include "remove.h"
#include "triangulate.h"

struct Polygon* MakePolygon(Arena *arena, size_t numVertices, Vec2 vertices[static numVertices])
{
    //struct Polygon *polygon = calloc(1, sizeof *polygon + numVertices * sizeof *polygon->vertices);
    struct Polygon *polygon = arena_alloc(arena, sizeof *polygon + numVertices * sizeof *polygon->vertices);
    for(size_t i = 0; i < numVertices; ++i)
    {
       polygon->vertices[i][0] = (double)vertices[i].x;
       polygon->vertices[i][1] = (double)vertices[i].y;
    }
    polygon->length = numVertices;
    return polygon;
}

SplitResult SplitMapLine(Map *map, MapLine *line, MapVertex *vertex)
{
    LineData dataCopy = CopyLineData(line->data);

    MapVertex *va = line->a;
    MapVertex *vb = line->b;

    RemoveLine(map, line);

    MapLine *newA = EditAddLine(map, va, vertex, dataCopy);
    MapLine *newB = EditAddLine(map, vertex, vb, dataCopy);

    FreeLineData(dataCopy);

    return (SplitResult){ .left = newA, .right = newB };
}

SplitResult SplitMapLine2(Map *map, MapLine *line, MapVertex *vertexA, MapVertex *vertexB)
{
    LineData dataCopy = CopyLineData(line->data);

    MapVertex *va = line->a;
    MapVertex *vb = line->b;

    RemoveLine(map, line);

    MapLine *newStart = EditAddLine(map, va, vertexA, dataCopy);
    MapLine *newMiddle = EditAddLine(map, vertexA, vertexB, dataCopy);
    MapLine *newEnd = EditAddLine(map, vertexB, vb, dataCopy);

    FreeLineData(dataCopy);

    return (SplitResult){ .left = newStart, .middle = newMiddle, .right = newEnd };
}

static polygon_t* FillPolygon(size_t numLines, MapLine *lines[static numLines], polygon_t *polygon)
{
	MapVertex *vertex, *nextVertex;
	GetLoopStart(lines[0], lines[1], &vertex, &nextVertex);
    polygon->vertices[0] = vertex->pos;
    for(size_t i = 1; i < numLines; ++i)
    {
        MapLine *mapLine = lines[i];
        bool front = mapLine->a == nextVertex;
        vertex = front ? mapLine->a : mapLine->b;
        nextVertex = front ? mapLine->b : mapLine->a;

        polygon->vertices[i] = vertex->pos;
    }
    return polygon;
}

polygon_t* PolygonFromMapLines(size_t numLines, MapLine *lines[static numLines])
{
    assert(numLines >= 3);
    polygon_t *polygon = calloc(1, sizeof *polygon + numLines * sizeof *polygon->vertices);
    polygon->length = numLines;
	return FillPolygon(numLines, lines, polygon);
}

polygon_t* PolygonFromMapLinesArena(Arena *arena, size_t numLines, MapLine *lines[static numLines])
{
    assert(numLines >= 3);
    polygon_t *polygon = arena_alloc(arena, sizeof *polygon + numLines * sizeof *polygon->vertices);
    polygon->length = numLines;
	return FillPolygon(numLines, lines, polygon);
}

polygon_t* PolygonFromVertices(size_t numVertices, EditorVertexType vertices[static numVertices])
{
    polygon_t *polygon = calloc(1, sizeof *polygon + numVertices * sizeof *polygon->vertices);
    polygon->length = numVertices;
    for(size_t i = 0; i < numVertices; ++i)
    {
        polygon->vertices[i] = vertices[i].position;
    }
    return polygon;
}

polygon_t* PolygonFromVectors(size_t numVectors, Vec2 vectors[static numVectors])
{
    polygon_t *polygon = calloc(1, sizeof *polygon + numVectors * sizeof *polygon->vertices);
    polygon->length = numVectors;
    for(size_t i = 0; i < numVectors; ++i)
    {
        polygon->vertices[i] = vectors[i];
    }
    return polygon;
}

bool IsLineFront(MapVertex *v1, MapLine *line)
{
    assert(v1 == line->a || v1 == line->b);
    return v1 == line->a;
}

MapLine* GetMapLine(Map *map, line_t line)
{
    for(MapLine *mline = map->headLine; mline; mline = mline->next)
    {
        if((vec2_eq(mline->a->pos, line.a) && vec2_eq(mline->b->pos, line.b)) ||
           (vec2_eq(mline->b->pos, line.a) && vec2_eq(mline->a->pos, line.b)))
           return mline;
    }
    return NULL;
}

void GetLoopStart(MapLine *l0, MapLine *l1, MapVertex **outVertex, MapVertex **outNext)
{
	if(l0->a == l1->a || l0->a == l1->b)
	{
		*outVertex = l0->b;
		*outNext = l0->a;
	}
	else
	{
		*outVertex = l0->a;
		*outNext = l0->b;
	}
}

