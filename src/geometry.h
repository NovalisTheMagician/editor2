#pragma once

#include <stdint.h>
#include <stdlib.h>
#include "map.h"

bool PointInSector(struct MapSector *sector, struct Vertex point);
bool PointInPolygon(struct Vertex *vertices, size_t numVertices, struct Vertex point);
