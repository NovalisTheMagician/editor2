#include "remove.h"

#include <string.h>

void RemoveVertex(Map *map, MapVertex *vertex)
{
    MapVertex *prev = vertex->prev;
    MapVertex *next = vertex->next;

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
        MapLine *line = vertex->attachedLines[i];
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

void RemoveLine(Map *map, MapLine *line)
{
    MapLine *prev = line->prev;
    MapLine *next = line->next;

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
        MapVertex *v = line->a;
        for(size_t i = line->aVertIndex + 1; i < v->numAttachedLines; ++i)
        {
            MapLine *attLine = v->attachedLines[i];
            attLine->a == v ? attLine->aVertIndex-- : attLine->bVertIndex--;
        }
        memmove(v->attachedLines + line->aVertIndex, v->attachedLines + line->aVertIndex + 1, (v->numAttachedLines - (line->aVertIndex)) * sizeof *v->attachedLines);
        v->numAttachedLines--;
    }

    if(line->b && line->b->numAttachedLines > 0)
    {
        MapVertex *v = line->b;
        for(size_t i = line->bVertIndex + 1; i < v->numAttachedLines; ++i)
        {
            MapLine *attLine = v->attachedLines[i];
            attLine->a == v ? attLine->aVertIndex-- : attLine->bVertIndex--;
        }
        memmove(v->attachedLines + line->bVertIndex, v->attachedLines + line->bVertIndex + 1, (v->numAttachedLines - (line->bVertIndex)) * sizeof *v->attachedLines);
        v->numAttachedLines--;
    }

    FreeMapLine(line);

    map->numLines--;
    map->dirty = true;
}

void RemoveSector(Map *map, MapSector *sector)
{
    MapSector *prev = sector->prev;
    MapSector *next = sector->next;

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
        MapLine *line = sector->outerLines[i];
        if(line->frontSector == sector) line->frontSector = NULL;
        if(line->backSector == sector) line->backSector = NULL;
    }

    for(size_t i = 0; i < sector->numInnerLines; ++i)
    {
        for(size_t j = 0; j < sector->numInnerLinesNum[i]; ++j)
        {
            MapLine *line = sector->innerLines[i][j];
            if(line->frontSector == sector) line->frontSector = NULL;
            if(line->backSector == sector) line->backSector = NULL;
        }
    }

    FreeMapSector(sector);

    map->numSectors--;
    map->dirty = true;
}
