#pragma once

#include <stdint.h>
#include <stdlib.h>
#include "map.h"

bool PointInSector(struct MapSector *sector, ivec2s point);
bool PointInPolygon(ivec2s *vertices, size_t numVertices, ivec2s point);
