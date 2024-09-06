#include "util.h"

#include <assert.h>

#include "../edit.h"
#include "map.h"
#include "remove.h"

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
