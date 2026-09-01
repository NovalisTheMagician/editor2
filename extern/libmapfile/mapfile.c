#include "mapfile.h"

#include <stdarg.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

static void DefaultLog(int severity, const char *msg)
{
    (void)severity;
    (void)msg;
}

static LogFunc Log = DefaultLog;

static void DoLog(int severity, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    static char buffer[1024] = { 0 };
    vsnprintf(buffer, sizeof buffer, format, args);

    Log(severity, buffer);

    va_end(args);
}

#define LINE_BUFFER_COUNT 2048
static char linebuffer[LINE_BUFFER_COUNT];

#define KEY_VERSION "version"
#define KEY_EDITOR "editor"
#define KEY_GRAVITY "gravity"
#define KEY_TEXTURESCALE "texture_scale"
#define KEY_VERTICES "vertices"
#define KEY_LINES "lines"
#define KEY_SECTORS "sectors"

#define KEY_IS(k) strcasecmp(key, k) == 0

typedef struct LinePart
{
    size_t offset;
    char *string;
} LinePart;

static char* Ltrim(char *s)
{
    while(isspace(*s)) s++;
    return s;
}

static char* Rtrim(char *s)
{
    size_t len = strlen(s);
    if(len == 0) return s;
    char* back = s + len;
    while(isspace(*--back));
    *(back+1) = '\0';
    return s;
}

static char* Trim(char *s)
{
    if(!s) return NULL;
    return Rtrim(Ltrim(s));
}

static bool ParseBool(char *str, bool *val)
{
    char *end;
    *val = strtol(str, &end, 10);
    return !(str == end);
}

static bool ParseIndex(char *str, size_t *val)
{
    char *end;
    *val = strtoull(str, &end, 10);
    return !(str == end);
}

static bool ParseInt(char *str, int *val)
{
    char *end;
    *val = strtol(str, &end, 10);
    return !(str == end);
}

static bool ParseUint(char *str, uint32_t *val)
{
    char *end;
    *val = strtoul(str, &end, 10);
    return !(str == end);
}

static bool ParseFloat(char *str, float *val)
{
    char *end;
    *val = strtof(str, &end);
    return !(str == end);
}

static bool ParseDouble(char *str, double *val)
{
    char *end;
    *val = strtod(str, &end);
    return !(str == end);
}

static bool ParseLineSide(LinePart *parts, MapfileLineSide *side, int lineNr, int charOffset)
{
    if(strcmp(parts[0].string, "NULL") == 0)
    {
        side->lowerTex = NULL;
        side->lowerTexLen = 0;
    }
    else
    {
        side->lowerTex = parts[0].string;
        side->lowerTexLen = strlen(side->lowerTex);
    }
    
    if(!ParseFloat(parts[1].string, &side->lowerOffsetX))
    {
        DoLog(MS_ERROR, "Failed to parse Line lower Texture-X-Offset at %d:%d", lineNr, parts[1].offset + charOffset);
        return false;
    }

    if(!ParseFloat(parts[2].string, &side->lowerOffsetY))
    {
        DoLog(MS_ERROR, "Failed to parse Line lower Texture-Y-Offset at %d:%d", lineNr, parts[2].offset + charOffset);
        return false;
    }

    if(strcmp(parts[3].string, "NULL") == 0)
    {
        side->middleTex = NULL;
        side->middleTexLen = 0;
    }
    else
    {
        side->middleTex = parts[3].string;
        side->middleTexLen = strlen(side->middleTex);
    }
    
    if(!ParseFloat(parts[4].string, &side->middleOffsetX))
    {
        DoLog(MS_ERROR, "Failed to parse Line middle Texture-X-Offset at %d:%d", lineNr, parts[4].offset + charOffset);
        return false;
    }

    if(!ParseFloat(parts[5].string, &side->middleOffsetY))
    {
        DoLog(MS_ERROR, "Failed to parse Line middle Texture-Y-Offset at %d:%d", lineNr, parts[5].offset + charOffset);
        return false;
    }

    if(strcmp(parts[6].string, "NULL") == 0)
    {
        side->upperTex = NULL;
        side->upperTexLen = 0;
    }
    else
    {
        side->upperTex = parts[6].string;
        side->upperTexLen = strlen(side->upperTex);
    }
    
    if(!ParseFloat(parts[7].string, &side->upperOffsetX))
    {
        DoLog(MS_ERROR, "Failed to parse Line upper Texture-X-Offset at %d:%d", lineNr, parts[7].offset + charOffset);
        return false;
    }

    if(!ParseFloat(parts[8].string, &side->upperOffsetY))
    {
        DoLog(MS_ERROR, "Failed to parse Line upper Texture-Y-Offset at %d:%d", lineNr, parts[8].offset + charOffset);
        return false;
    }
    return true;
}

MAPFILEDEF void MapfileSetLogFunc(LogFunc logfunc)
{
    if(logfunc)
        Log = logfunc;
    else
        Log = DefaultLog;
}

MAPFILEDEF bool MapfileParse(FILE *file, Mapfile *mapfile, int flags)
{
    mapfile->file = file;
    mapfile->verticesBegin = mapfile->linesBegin = mapfile->sectorsBegin = 0;
    int lineNr = 0;
    bool inBlock = false;
    long *arrayEnd, lineStart = 0, prevLine = 0;
    while(fgets(linebuffer, sizeof linebuffer, file))
    {
        lineNr++;
        prevLine = lineStart;
        lineStart = ftell(file);
        char *line = Trim(linebuffer);
        if(*line == '\0') continue;
        if(!inBlock)
        {
            char *delim = strchr(line, '=');
            if(!delim || delim == line)
            {
                DoLog(MS_ERROR, "Mapfile parse error on line %d", lineNr);
                return false;
            }

            *delim = '\0';
            char *key = Trim(line);
            char *value = Trim(delim + 1);

            if(KEY_IS(KEY_VERSION))
            {
                int version;
                if(!ParseInt(value, &version))
                {
                    DoLog(MS_ERROR, "Unable to parse Version of mapfile");
                    return false;
                }
                if(version != MAPFILE_VERSION)
                {
                    DoLog(MS_ERROR, "Unsuported version of mapfile (got %d, expected %d)", version, MAPFILE_VERSION);
                    return false;
                }
            }

            if(KEY_IS(KEY_GRAVITY))
            {
                if(!ParseFloat(value, &mapfile->gravity))
                {
                    DoLog(MS_WARNING, "Failed to parse gravity: Using default value %f", 9.8f);
                    mapfile->gravity = 9.8f;
                }
            }

            if(KEY_IS(KEY_TEXTURESCALE))
            {
                if(!ParseInt(value, &mapfile->textureScale))
                {
                    DoLog(MS_WARNING, "Failed to parse textureScale: Using default value %d", 1);
                    mapfile->textureScale = 1;
                }
            }

            if(KEY_IS(KEY_VERTICES))
            {
                if(strcmp(value, "{") != 0)
                {
                    DoLog(MS_ERROR, "Missing { for vertices array at %d", lineNr);
                    return false;
                }
                inBlock = true;
                mapfile->verticesBegin = lineStart;
                arrayEnd = &mapfile->verticesEnd;
                mapfile->verticesStartLineNr = lineNr+1;
            }

            if(KEY_IS(KEY_LINES))
            {
                if(strcmp(value, "{") != 0)
                {
                    DoLog(MS_ERROR, "Missing { for lines array at %d", lineNr);
                    return false;
                }
                inBlock = true;
                mapfile->linesBegin = lineStart;
                arrayEnd = &mapfile->linesEnd;
                mapfile->linesStartLineNr = lineNr+1;
            }

            if(KEY_IS(KEY_SECTORS))
            {
                if(strcmp(value, "{") != 0)
                {
                    DoLog(MS_ERROR, "Missing { for sectors array at %d", lineNr);
                    return false;
                }
                inBlock = true;
                mapfile->sectorsBegin = lineStart;
                arrayEnd = &mapfile->sectorsEnd;
                mapfile->sectorsStartLineNr = lineNr+1;
            }
        }
        else
        {
            if(strcmp(line, "}") == 0)
            {
                inBlock = false;
                *arrayEnd = prevLine;
                continue;
            }
        }
    }
    return true;
}

MAPFILEDEF bool MapfileIteratorEnd(MapfileIterator *iterator)
{
    return iterator->lineLoc == 0;
}

static char* GetNextLine(long from, long *next, int *lineNr, FILE *file, long end)
{
    if(from == 0)
        return NULL;

    int ret = fseek(file, from, SEEK_SET);
    if(ret == -1)
    {
        DoLog(MS_ERROR, "Failed to parse Vertex at %d: %s", *lineNr, strerror(errno));
        return NULL;
    }

    char *line = fgets(linebuffer, sizeof linebuffer, file);
    if(!line)
    {
        *next = 0;
        DoLog(MS_ERROR, "Failed to parse Vertex at %d", *lineNr);
        return NULL;
    }
    *next = ftell(file);
    if(*next == end)
        *next = 0;
    *lineNr++;
    return line;
}

static size_t GetLineParts(char *line, size_t maxParts, LinePart parts[static maxParts])
{
    assert(maxParts > 0);
    size_t current = 0;
    LinePart *part = &parts[current++];
    part->string = line;
    part->offset = 0;
    char *delim = strchr(line, ' ');
    while(delim)
    {
        part = &parts[current++];
        part->string = delim + 1;
        part->offset = delim - line;
        *delim = '\0';
        if(current == maxParts)
            break;
        delim = strchr(delim + 1, ' ');
    }
    return current;
}

MAPFILEDEF MapfileIterator MapfileVerticesIterator(Mapfile *mapfile)
{
    long loc = mapfile->verticesBegin, nextLoc = 0;
    int lineNr = mapfile->verticesStartLineNr; 
    long end = mapfile->verticesEnd;
    char *line = GetNextLine(loc, &nextLoc, &lineNr, mapfile->file, end);
    return (MapfileIterator){ .type = IT_VERTEX, .lineLoc = loc, .nextLoc = nextLoc, .line = line, .lineNr = lineNr, .mapfile = mapfile };
}

MAPFILEDEF void MapfileIteratorNextVertex(MapfileIterator *iterator)
{
    assert(iterator->type == IT_VERTEX);

    iterator->lineLoc = iterator->nextLoc;
    iterator->line = GetNextLine(iterator->lineLoc, &iterator->nextLoc, &iterator->lineNr, iterator->mapfile->file, iterator->mapfile->verticesEnd);
}

MAPFILEDEF bool MapfileIteratorGetVertex(MapfileIterator *iterator, MapfileVertex *vertex)
{
    assert(iterator->type == IT_VERTEX);

    int lineNr = iterator->lineNr;
    char *line = iterator->line;

    LinePart parts[32] = { 0 };
    size_t numParts = GetLineParts(Trim(line), 32, parts);

    if(numParts < 3)
    {
        DoLog(MS_ERROR, "Failed to parse Vertex at %d: Not enough data (expected 3 got %d)", lineNr, numParts);
        return false;
    }

    size_t whiteSpaces = parts[0].string - line;
    if(!ParseIndex(parts[0].string, &vertex->index))
    {
        DoLog(MS_ERROR, "Failed to parse Vertex Index at %d:%d", lineNr, parts[0].offset + whiteSpaces);
        return false;
    }

    if(!ParseFloat(parts[1].string, &vertex->x))
    {
        DoLog(MS_ERROR, "Failed to parse Vertex X-Coord at %d:%d", lineNr, parts[1].offset + whiteSpaces);
        return false;
    }

    if(!ParseFloat(parts[2].string, &vertex->y))
    {
        DoLog(MS_ERROR, "Failed to parse Vertex Y-Coord at %d:%d", lineNr, parts[2].offset + whiteSpaces);
        return false;
    }

    return true;
}

MAPFILEDEF MapfileIterator MapfileLinesIterator(Mapfile *mapfile)
{
    long loc = mapfile->linesBegin, nextLoc = 0;
    int lineNr = mapfile->linesStartLineNr;
    long end = mapfile->linesEnd;
    char *line = GetNextLine(loc, &nextLoc, &lineNr, mapfile->file, end);
    return (MapfileIterator){ .type = IT_LINE, .lineLoc = loc, .nextLoc = nextLoc, .line = line, .lineNr = lineNr, .mapfile = mapfile };
}

MAPFILEDEF void MapfileIteratorNextLine(MapfileIterator *iterator)
{
    assert(iterator->type == IT_LINE);

    iterator->lineLoc = iterator->nextLoc;
    iterator->line = GetNextLine(iterator->lineLoc, &iterator->nextLoc, &iterator->lineNr, iterator->mapfile->file, iterator->mapfile->linesEnd);
}

MAPFILEDEF bool MapfileIteratorGetLine(MapfileIterator *iterator, MapfileLine *mapline)
{
    assert(iterator->type == IT_LINE);

    int lineNr = iterator->lineNr;
    char *line = iterator->line;

    LinePart parts[32] = { 0 };
    size_t numParts = GetLineParts(Trim(line), 32, parts);

    if(numParts < 22)
    {
        DoLog(MS_ERROR, "Failed to parse Line at %d: Not enough data (expected 22 got %d)", lineNr, numParts);
        return false;
    }

    size_t whiteSpaces = parts[0].string - line;
    if(!ParseIndex(parts[0].string, &mapline->index))
    {
        DoLog(MS_ERROR, "Failed to parse Line index at %d:%d", lineNr, parts[0].offset + whiteSpaces);
        return false;
    }

    if(!ParseIndex(parts[1].string, &mapline->vertexA))
    {
        DoLog(MS_ERROR, "Failed to parse Line A-Vertex at %d:%d", lineNr, parts[1].offset + whiteSpaces);
        return false;
    }

    if(!ParseIndex(parts[2].string, &mapline->vertexB))
    {
        DoLog(MS_ERROR, "Failed to parse Line B-Vertex at %d:%d", lineNr, parts[2].offset + whiteSpaces);
        return false;
    }

    uint32_t flags;
    if(!ParseUint(parts[3].string, &flags))
    {
        DoLog(MS_ERROR, "Failed to parse Line A-Vertex at %d:%d", lineNr, parts[3].offset + whiteSpaces);
        return false;
    }

    if(!ParseLineSide(parts + 4, &mapline->front, lineNr, whiteSpaces))
        return false;

    if(!ParseLineSide(parts + 4 + 9, &mapline->back, lineNr, whiteSpaces))
        return false;

    return true;
}

MAPFILEDEF MapfileIterator MapfileSectorsIterator(Mapfile *mapfile)
{
    long loc = mapfile->sectorsBegin, nextLoc = 0;
    int lineNr = mapfile->sectorsStartLineNr;
    long end = mapfile->sectorsEnd;
    char *line = GetNextLine(loc, &nextLoc, &lineNr, mapfile->file, end);
    return (MapfileIterator){ .type = IT_SECTOR, .lineLoc = loc, .nextLoc = nextLoc, .line = line, .lineNr = lineNr, .mapfile = mapfile };
}

MAPFILEDEF void MapfileIteratorNextSector(MapfileIterator *iterator)
{
    assert(iterator->type == IT_SECTOR);

    iterator->lineLoc = iterator->nextLoc;
    iterator->line = GetNextLine(iterator->lineLoc, &iterator->nextLoc, &iterator->lineNr, iterator->mapfile->file, iterator->mapfile->sectorsEnd);
}

MAPFILEDEF bool MapfileIteratorGetSector(MapfileIterator *iterator, MapfileSector *sector)
{
    assert(iterator->type == IT_SECTOR);

    int lineNr = iterator->lineNr;
    char *line = iterator->line;

    LinePart parts[32] = { 0 };
    size_t numParts = GetLineParts(Trim(line), 32, parts);

    if(numParts < 13)
    {
        DoLog(MS_ERROR, "Failed to parse Sector at %d: Not enough data (expected 13 got %d)", lineNr, numParts);
        return false;
    }

    size_t whiteSpaces = parts[0].string - line;
    if(!ParseIndex(parts[0].string, &sector->index))
    {
        DoLog(MS_ERROR, "Failed to parse Sector index at %d:%d", lineNr, parts[0].offset + whiteSpaces);
        return false;
    }

    if(!ParseIndex(parts[1].string, &sector->firstLine))
    {
        DoLog(MS_ERROR, "Failed to parse Sector first line at %d:%d", lineNr, parts[1].offset + whiteSpaces);
        return false;
    }

    if(!ParseBool(parts[2].string, &sector->firstLineFront))
    {
        DoLog(MS_ERROR, "Failed to parse Sector first line front at %d:%d", lineNr, parts[2].offset + whiteSpaces);
        return false;
    }

    if(!ParseInt(parts[3].string, &sector->floorHeight))
    {
        DoLog(MS_ERROR, "Failed to parse Sector floor height at %d:%d", lineNr, parts[3].offset + whiteSpaces);
        return false;
    }

    if(!ParseInt(parts[4].string, &sector->ceilHeight))
    {
        DoLog(MS_ERROR, "Failed to parse Sector ceiling height at %d:%d", lineNr, parts[4].offset + whiteSpaces);
        return false;
    }

    uint32_t flags;
    if(!ParseUint(parts[5].string, &flags))
    {
        DoLog(MS_ERROR, "Failed to parse Sector flags at %d:%d", lineNr, parts[5].offset + whiteSpaces);
        return false;
    }

    if(!ParseFloat(parts[6].string, &sector->floorOffsetX))
    {
        DoLog(MS_ERROR, "Failed to parse Sector floor Offset-X at %d:%d", lineNr, parts[6].offset + whiteSpaces);
        return false;
    }

    if(!ParseFloat(parts[7].string, &sector->floorOffsetY))
    {
        DoLog(MS_ERROR, "Failed to parse Sector floor Offset-Y at %d:%d", lineNr, parts[7].offset + whiteSpaces);
        return false;
    }

    if(!ParseFloat(parts[8].string, &sector->ceilOffsetX))
    {
        DoLog(MS_ERROR, "Failed to parse Sector floor Offset-X at %d:%d", lineNr, parts[8].offset + whiteSpaces);
        return false;
    }

    if(!ParseFloat(parts[9].string, &sector->ceilOffsetY))
    {
        DoLog(MS_ERROR, "Failed to parse Sector floor Offset-Y at %d:%d", lineNr, parts[9].offset + whiteSpaces);
        return false;
    }

    int lightLevel;
    if(!ParseInt(parts[10].string, &lightLevel))
    {
        DoLog(MS_ERROR, "Failed to parse Sector lightlevel at %d:%d", lineNr, parts[10].offset + whiteSpaces);
        return false;
    }
    if(lightLevel > 255)
    {
        DoLog(MS_WARNING, "Sector Lightlevel exceeds maximum value. Clamping it to 255 (%d:%d)", lineNr, parts[10].offset + whiteSpaces);
        lightLevel = 255;
    }
    sector->lightLevel = lightLevel;

    sector->floorTex = parts[11].string;
    sector->floorTexLen = strlen(parts[11].string);

    sector->ceilTex = parts[12].string;
    sector->ceilTexLen = strlen(parts[12].string);

    return true;
}

MAPFILEDEF void MapfileBeginWrite(FILE *file)
{
}

MAPFILEDEF void MapfileEndWrite(FILE *file)
{
}

MAPFILEDEF void MapfileWriteGravity(FILE *file, float gravity)
{
}

MAPFILEDEF void MapfileWriteEditor(FILE *file, const char *editorName, size_t editorNameCount)
{
}

MAPFILEDEF void MapfileWriteTextureScale(FILE *file, int textureScale)
{
}

MAPFILEDEF void MapfileWriteBeginVertices(FILE *file)
{
}

MAPFILEDEF void MapfileWriteEndVertices(FILE *file)
{
}

MAPFILEDEF void MapfileWriteVertex(FILE *file, size_t idx, float x, float y)
{
}

MAPFILEDEF void MapfileWriteBeginLines(FILE *file)
{
}

MAPFILEDEF void MapfileWriteEndLines(FILE *file)
{
}

MAPFILEDEF void MapfileWriteLine(FILE *file, size_t idx, size_t aIdx, size_t bIdx, uint32_t flags, ...)
{
}

MAPFILEDEF void MapfileWriteBeginSectors(FILE *file)
{
}

MAPFILEDEF void MapfileWriteEndSectors(FILE *file)
{
}


