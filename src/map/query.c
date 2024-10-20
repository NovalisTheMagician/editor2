#include "query.h"
#include "map.h"

MapVertex* GetVertex(Map *map, size_t idx)
{
    for(MapVertex *vertex = map->headVertex; vertex; vertex = vertex->next)
        if(vertex->idx == idx)
            return vertex;
    return NULL;
}

MapLine* GetLine(Map *map, size_t idx)
{
    for(MapLine *line = map->headLine; line; line = line->next)
        if(line->idx == idx)
            return line;
    return NULL;
}

MapSector* GetSector(Map *map, size_t idx)
{
    for(MapSector *sector = map->headSector; sector; sector = sector->next)
        if(sector->idx == idx)
            return sector;
    return NULL;
}
