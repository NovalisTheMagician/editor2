#include "remove.h"
#include "map/query.h"

#include <string.h>
#include <assert.h>
#include <sys/types.h>

#define deleteElement(list, element) do {   ptrdiff_t arrIndex = (element) - ((list).items); \
                                            assert(arrIndex >= 0 && arrIndex < (ptrdiff_t)(list).count); \
                                            memmove((list).items + arrIndex, (list).items + arrIndex + 1, ((list).count - (arrIndex)) * sizeof *((list).items)); \
                                            (list).count--; \
                                        } while(0)

void RemoveVertex(Map *map, MapVertex *vertex)
{
    FreeMapVertex(vertex);
    deleteElement(map->vertexList, vertex);
    map->dirty = true;
}

void RemoveLine(Map *map, MapLine *line)
{
    MapVertex *a = GetVertex(map, line->a);
    MapVertex *b = GetVertex(map, line->b);

    if(a && a->numAttachedLines > 0)
    {
        MapVertex *v = a;
        for(size_t i = line->aVertIndex + 1; i < v->numAttachedLines; ++i)
        {
            MapLine *attLine = GetLine(map, v->attachedLines[i]);
            attLine->a == v->idx ? attLine->aVertIndex-- : attLine->bVertIndex--;
        }
        memmove(v->attachedLines + line->aVertIndex, v->attachedLines + line->aVertIndex + 1, (v->numAttachedLines - (line->aVertIndex)) * sizeof *v->attachedLines);
        v->numAttachedLines--;
    }

    if(b && b->numAttachedLines > 0)
    {
        MapVertex *v = b;
        for(size_t i = line->bVertIndex + 1; i < v->numAttachedLines; ++i)
        {
            MapLine *attLine = GetLine(map, v->attachedLines[i]);
            attLine->a == v->idx ? attLine->aVertIndex-- : attLine->bVertIndex--;
        }
        memmove(v->attachedLines + line->bVertIndex, v->attachedLines + line->bVertIndex + 1, (v->numAttachedLines - (line->bVertIndex)) * sizeof *v->attachedLines);
        v->numAttachedLines--;
    }

    FreeMapLine(line);
    deleteElement(map->lineList, line);
    map->dirty = true;
}

void RemoveSector(Map *map, MapSector *sector)
{
    for(size_t i = 0; i < sector->numOuterLines; ++i)
    {
        MapLine *line = GetLine(map, sector->outerLines[i]);
        if(line->frontSector == (ssize_t)sector->idx) line->frontSector = -1;
        if(line->backSector == (ssize_t)sector->idx) line->backSector = -1;
    }

    for(size_t i = 0; i < sector->numInnerLines; ++i)
    {
        for(size_t j = 0; j < sector->numInnerLinesNum[i]; ++j)
        {
            MapLine *line = GetLine(map, sector->innerLines[i][j]);
            if(line->frontSector == (ssize_t)sector->idx) line->frontSector = -1;
            if(line->backSector == (ssize_t)sector->idx) line->backSector = -1;
        }
    }

    FreeMapSector(sector);
    deleteElement(map->sectorList, sector);
    map->dirty = true;
}
