#include "util.h"

#include <assert.h>

#include "../edit.h"
#include "remove.h"

struct SplitResult SplitMapLine(struct EdState state[static 1], struct MapLine line[static 1], struct MapVertex vertex[static 1])
{
    struct Map *map = &state->map;

    struct LineData dataCopy = line->data;

    struct MapVertex *va = line->a;
    struct MapVertex *vb = line->b;

    RemoveLine(map, line);

    struct MapLine *newA = EditAddLine(state, va, vertex, dataCopy);
    struct MapLine *newB = EditAddLine(state, vertex, vb, dataCopy);

    return (struct SplitResult){ .left = newA, .right = newB };
}

struct SplitResult SplitMapLine2(struct EdState state[static 1], struct MapLine line[static 1], struct MapVertex vertexA[static 1], struct MapVertex vertexB[static 1])
{
    struct Map *map = &state->map;

    struct LineData dataCopy = line->data;

    struct MapVertex *va = line->a;
    struct MapVertex *vb = line->b;

    RemoveLine(map, line);

    struct MapLine *newStart = EditAddLine(state, va, vertexA, dataCopy);
    struct MapLine *newMiddle = EditAddLine(state, vertexA, vertexB, dataCopy);
    struct MapLine *newEnd = EditAddLine(state, vertexB, vb, dataCopy);

    return (struct SplitResult){ .left = newStart, .middle = newMiddle, .right = newEnd };
}

struct Polygon* PolygonFromMapLines(size_t numLines, struct MapLine *lines[static numLines], bool lineFronts[static numLines])
{
    struct Polygon *polygon = calloc(1, sizeof *polygon + numLines * sizeof *polygon->vertices);
    polygon->length = numLines;

    for(size_t i = 0; i < numLines; ++i)
    {
        struct MapLine *mapLine = lines[i];
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

bool IsLineFront(struct MapVertex *v1, struct MapLine *line)
{
    assert(v1 == line->a || v1 == line->b);
    return v1 == line->a;
}

struct MapLine* GetMapLine(struct Map map[static 1], struct line_t line)
{
    for(struct MapLine *mline = map->headLine; mline; mline = mline->next)
    {
        if((glms_vec2_eqv_eps(mline->a->pos, line.a) && glms_vec2_eqv_eps(mline->b->pos, line.b)) ||
           (glms_vec2_eqv_eps(mline->b->pos, line.a) && glms_vec2_eqv_eps(mline->a->pos, line.b)))
           return mline;
    }
    return NULL;
}
