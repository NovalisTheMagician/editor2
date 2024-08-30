#include "util.h"

#include <assert.h>

#include "../edit.h"
#include "remove.h"

SplitResult SplitMapLine(EdState *state, MapLine *line, MapVertex *vertex)
{
    Map *map = &state->map;

    LineData dataCopy = line->data;

    MapVertex *va = line->a;
    MapVertex *vb = line->b;

    RemoveLine(map, line);

    MapLine *newA = EditAddLine(state, va, vertex, dataCopy);
    MapLine *newB = EditAddLine(state, vertex, vb, dataCopy);

    return (SplitResult){ .left = newA, .right = newB };
}

SplitResult SplitMapLine2(EdState *state, MapLine *line, MapVertex *vertexA, MapVertex *vertexB)
{
    Map *map = &state->map;

    LineData dataCopy = line->data;

    MapVertex *va = line->a;
    MapVertex *vb = line->b;

    RemoveLine(map, line);

    MapLine *newStart = EditAddLine(state, va, vertexA, dataCopy);
    MapLine *newMiddle = EditAddLine(state, vertexA, vertexB, dataCopy);
    MapLine *newEnd = EditAddLine(state, vertexB, vb, dataCopy);

    return (SplitResult){ .left = newStart, .middle = newMiddle, .right = newEnd };
}

struct Polygon* PolygonFromMapLines(size_t numLines, MapLine *lines[static numLines], bool lineFronts[static numLines])
{
    struct Polygon *polygon = calloc(1, sizeof *polygon + numLines * sizeof *polygon->vertices);
    polygon->length = numLines;

    for(size_t i = 0; i < numLines; ++i)
    {
        MapLine *mapLine = lines[i];
        if(lineFronts[i])
        {
            polygon->vertices[i][0] = mapLine->a->pos.x;
            polygon->vertices[i][1] = mapLine->a->pos.y;
        }
        else
        {
            polygon->vertices[i][0] = mapLine->b->pos.x;
            polygon->vertices[i][1] = mapLine->b->pos.y;
        }
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
        if((glms_vec2_eqv_eps(mline->a->pos, line.a) && glms_vec2_eqv_eps(mline->b->pos, line.b)) ||
           (glms_vec2_eqv_eps(mline->b->pos, line.a) && glms_vec2_eqv_eps(mline->a->pos, line.b)))
           return mline;
    }
    return NULL;
}
