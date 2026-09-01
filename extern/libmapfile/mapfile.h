#ifndef MAPFILE_H_

#define MAPFILE_VERSION 2

#ifndef MAPFILEDEF
#define MAPFILEDEF extern
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef struct MapfileVertex
{
    size_t index;
    float x, y;
} MapfileVertex;

typedef struct MapfileLineSide
{
    char *upperTex; size_t upperTexLen;
    float upperOffsetX, upperOffsetY;
    char *middleTex; size_t middleTexLen;
    float middleOffsetX, middleOffsetY;
    char *lowerTex; size_t lowerTexLen;
    float lowerOffsetX, lowerOffsetY;
} MapfileLineSide;

typedef struct MapfileLine
{
    size_t index;
    size_t vertexA, vertexB;
    MapfileLineSide front, back;
} MapfileLine;

typedef struct MapfileSector
{
    size_t index;
    size_t firstLine;
    bool firstLineFront;
    int32_t floorHeight, ceilHeight;
    float floorOffsetX, floorOffsetY;
    float ceilOffsetX, ceilOffsetY;
    uint8_t lightLevel;
    char *floorTex; size_t floorTexLen;
    char *ceilTex; size_t ceilTexLen;
} MapfileSector;

typedef struct Mapfile
{
    FILE *file;

    float gravity;
    int textureScale;

    long verticesBegin, verticesEnd;
    int verticesStartLineNr;
    long linesBegin, linesEnd;
    int linesStartLineNr;
    long sectorsBegin, sectorsEnd;
    int sectorsStartLineNr;
} Mapfile;

typedef enum IteratorType
{
    IT_VERTEX,
    IT_LINE,
    IT_SECTOR
} MapfileIteratorType;

typedef struct MapfileIterator
{
    MapfileIteratorType type;
    char *line;
    long lineLoc, nextLoc;
    int lineNr;
    Mapfile *mapfile;
} MapfileIterator;

typedef enum MapfileSeverity
{
    MS_INFO,
    MS_WARNING,
    MS_ERROR,
} MapfileSeverity;

typedef void(*LogFunc)(int, const char*);

MAPFILEDEF void MapfileSetLogFunc(LogFunc logfunc);
MAPFILEDEF bool MapfileParse(FILE *file, Mapfile *mapfile, int flags);

MAPFILEDEF bool MapfileIteratorEnd(MapfileIterator *iterator);

MAPFILEDEF MapfileIterator MapfileVerticesIterator(Mapfile *mapfile);
MAPFILEDEF void MapfileIteratorNextVertex(MapfileIterator *iterator);
MAPFILEDEF bool MapfileIteratorGetVertex(MapfileIterator *iterator, MapfileVertex *vertex);

MAPFILEDEF MapfileIterator MapfileLinesIterator(Mapfile *mapfile);
MAPFILEDEF void MapfileIteratorNextLine(MapfileIterator *iterator);
MAPFILEDEF bool MapfileIteratorGetLine(MapfileIterator *iterator, MapfileLine *line);

MAPFILEDEF MapfileIterator MapfileSectorsIterator(Mapfile *mapfile);
MAPFILEDEF void MapfileIteratorNextSector(MapfileIterator *iterator);
MAPFILEDEF bool MapfileIteratorGetSector(MapfileIterator *iterator, MapfileSector *sector);

MAPFILEDEF void MapfileBeginWrite(FILE *file);
MAPFILEDEF void MapfileEndWrite(FILE *file);
MAPFILEDEF void MapfileWriteGravity(FILE *file, float gravity);
MAPFILEDEF void MapfileWriteEditor(FILE *file, const char *editorName, size_t editorNameCount);
MAPFILEDEF void MapfileWriteTextureScale(FILE *file, int textureScale);

MAPFILEDEF void MapfileWriteBeginVertices(FILE *file);
MAPFILEDEF void MapfileWriteEndVertices(FILE *file);
MAPFILEDEF void MapfileWriteVertex(FILE *file, size_t idx, float x, float y);

MAPFILEDEF void MapfileWriteBeginLines(FILE *file);
MAPFILEDEF void MapfileWriteEndLines(FILE *file);
MAPFILEDEF void MapfileWriteLine(FILE *file, size_t idx, size_t aIdx, size_t bIdx, uint32_t flags, ...);

MAPFILEDEF void MapfileWriteBeginSectors(FILE *file);
MAPFILEDEF void MapfileWriteEndSectors(FILE *file);

#endif
