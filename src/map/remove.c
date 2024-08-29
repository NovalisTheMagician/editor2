#include "remove.h"

#include <string.h>

void RemoveVertex(struct Map *map, struct MapVertex *vertex)
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

    for(size_t i = 0; i < vertex->numAttachedLines; ++i)
    {
        struct MapLine *line = vertex->attachedLines[i];
        if(line->a == vertex)
        {
            line->a = NULL;
        }
        else
        {
            line->b = NULL;
        }
    }

    FreeMapVertex(vertex);

    map->numVertices--;
    map->dirty = true;
}

void RemoveLine(struct Map *map, struct MapLine *line)
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

    if(line->a && line->a->numAttachedLines > 0)
    {
        struct MapVertex *v = line->a;
        for(size_t i = line->aVertIndex + 1; i < v->numAttachedLines; ++i)
        {
            struct MapLine *attLine = v->attachedLines[i];
            attLine->a == v ? attLine->aVertIndex-- : attLine->bVertIndex--;
        }
        memmove(v->attachedLines + line->aVertIndex, v->attachedLines + line->aVertIndex + 1, (v->numAttachedLines - (line->aVertIndex)) * sizeof *v->attachedLines);
        v->numAttachedLines--;
        //memset(v->attachedLines + v->numAttachedLines, 0, COUNT_OF(v->attachedLines) - v->numAttachedLines);
    }

    if(line->b && line->b->numAttachedLines > 0)
    {
        struct MapVertex *v = line->b;
        for(size_t i = line->bVertIndex + 1; i < v->numAttachedLines; ++i)
        {
            struct MapLine *attLine = v->attachedLines[i];
            attLine->a == v ? attLine->aVertIndex-- : attLine->bVertIndex--;
        }
        memmove(v->attachedLines + line->bVertIndex, v->attachedLines + line->bVertIndex + 1, (v->numAttachedLines - (line->bVertIndex)) * sizeof *v->attachedLines);
        v->numAttachedLines--;
        //memset(v->attachedLines + v->numAttachedLines, 0, COUNT_OF(v->attachedLines) - v->numAttachedLines);
    }

    FreeMapLine(line);

    map->numLines--;
    map->dirty = true;
}

void RemoveSector(struct Map *map, struct MapSector *sector)
{
    struct MapSector *prev = sector->prev;
    struct MapSector *next = sector->next;

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
        // RemoveLine(state, sector->outerLines[i]);
        struct MapLine *line = sector->outerLines[i];
        if(line->frontSector == sector) line->frontSector = NULL;
        if(line->backSector == sector) line->backSector = NULL;
    }

    for(size_t i = 0; i < sector->numInnerLines; ++i)
    {
        for(size_t j = 0; j < sector->numInnerLinesNum[i]; ++j)
        {
            //RemoveLine(state, sector->innerLines[i][j]);
            struct MapLine *line = sector->innerLines[i][j];
            if(line->frontSector == sector) line->frontSector = NULL;
            if(line->backSector == sector) line->backSector = NULL;
        }
    }
    FreeMapSector(sector);

    map->numSectors--;
    map->dirty = true;
}
