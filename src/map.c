#include "map.h"

#include "logging.h"
#include "map/create.h"
#include "map/query.h"
#include "memory.h"
#include "utils/pstring.h"

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <strings.h>

static void FreeVertList(MapVertex *head)
{
    while(head)
    {
        MapVertex *vertex = head;
        head = head->next;

        FreeMapVertex(vertex);
    }
}

static void FreeLineList(MapLine *head)
{
    while(head)
    {
        MapLine *line = head;
        head = head->next;

        FreeMapLine(line);
    }
}

static void FreeSectorList(MapSector *head)
{
    while(head)
    {
        MapSector *sector = head;
        head = head->next;

        FreeMapSector(sector);
    }
}

LineData DefaultLineData(void)
{
    return (LineData){ .type = LT_NORMAL };
}

SectorData DefaultSectorData(void)
{
    return (SectorData){ .type = ST_NORMAL };
}

LineData CopyLineData(LineData data)
{
    LineData copy = data;
    if(data.front.lowerTex)
        copy.front.lowerTex = string_copy(data.front.lowerTex);
    if(data.front.middleTex)
        copy.front.middleTex = string_copy(data.front.middleTex);
    if(data.front.upperTex)
        copy.front.upperTex = string_copy(data.front.upperTex);
    if(data.back.lowerTex)
        copy.back.lowerTex = string_copy(data.back.lowerTex);
    if(data.back.middleTex)
        copy.back.middleTex = string_copy(data.back.middleTex);
    if(data.back.upperTex)
        copy.back.upperTex = string_copy(data.back.upperTex);
    return copy;
}

SectorData CopySectorData(SectorData data)
{
    SectorData copy = data;
    if(data.ceilTex)
        copy.ceilTex = string_copy(data.ceilTex);
    if(data.floorTex)
        copy.floorTex = string_copy(data.floorTex);
    return copy;
}

void FreeLineData(LineData data)
{
    string_free(data.front.lowerTex);
    string_free(data.front.middleTex);
    string_free(data.front.upperTex);
    string_free(data.back.lowerTex);
    string_free(data.back.middleTex);
    string_free(data.back.upperTex);
}

void FreeSectorData(SectorData data)
{
    string_free(data.ceilTex);
    string_free(data.floorTex);
}

void FreeMapVertex(MapVertex *vertex)
{
    free(vertex);
}

void FreeMapLine(MapLine *line)
{
    FreeLineData(line->data);
    free(line);
}

void FreeMapSector(MapSector *sector)
{
    FreeSectorData(sector->data);

    free(sector->outerLines);
    for(size_t i = 0; i < sector->numInnerLines; ++i)
        free(sector->innerLines[i]);
    free(sector->innerLines);

    free(sector->edData.vertices);
    free(sector->edData.indices);

    free(sector->contains);

    free(sector);
}

void NewMap(Map *map)
{
    FreeVertList(map->headVertex);
    map->headVertex = map->tailVertex = NULL;
    map->numVertices = 0;
    map->vertexIdx = 0;

    FreeLineList(map->headLine);
    map->headLine = map->tailLine = NULL;
    map->numLines = 0;
    map->lineIdx = 0;

    FreeSectorList(map->headSector);
    map->headSector = map->tailSector = NULL;
    map->numSectors = 0;
    map->sectorIdx = 0;

    string_free(map->file);
    map->file = string_alloc(1);

    map->dirty = false;

    map->textureScale = 1;
    map->gravity = 9.80f;
}

static char* ltrim(char *s)
{
    while(isspace(*s)) s++;
    return s;
}

static char* rtrim(char *s)
{
    char* back = s + strlen(s);
    while(isspace(*--back));
    *(back+1) = '\0';
    return s;
}

static char* trim(char *s)
{
    return rtrim(ltrim(s));
}

enum ParseMode
{
    PARSE_PROPS,
    PARSE_VERTICES,
    PARSE_LINES,
    PARSE_SECTORS
};

static char* parseIndex(char *line, size_t *idx)
{
    char *delim = strchr(line, ' ');
    if(!delim)
    {
        return NULL;
    }
    *delim = '\0';
    errno = 0;
    char *end;
    *idx = strtoull(line, &end, 10);
    if(line == end)
    {
        LogError("Failed to parse index");
        return NULL;
    }
    return delim+1;
}

static char* parseUint(char *line, uint32_t *i)
{
    char *delim = strchr(line, ' ');
    if(!delim)
    {
        return NULL;
    }
    *delim = '\0';
    errno = 0;
    char *end;
    *i = strtoul(line, &end, 10);
    if(line == end)
    {
        LogError("Failed to parse uint");
        return NULL;
    }
    return delim+1;
}

static char* parseInt(char *line, int *i)
{
    char *delim = strchr(line, ' ');
    if(!delim)
    {
        return NULL;
    }
    *delim = '\0';
    errno = 0;
    char *end;
    *i = strtol(line, &end, 10);
    if(line == end)
    {
        LogError("Failed to parse int");
        return NULL;
    }
    return delim+1;
}

static char* parseFloat(char *line, float *f)
{
    char *delim = strchr(line, ' ');
    if(!delim)
    {
        return NULL;
    }
    *delim = '\0';
    errno = 0;
    char *end;
    *f = strtof(line, &end);
    if(line == end)
    {
        LogError("Failed to parse float");
        return NULL;
    }
    return delim+1;
}

static char* parseString(char *line, char **str)
{
    char *delim = strchr(line, ' ');
    if(!delim)
    {
        return NULL;
    }
    *delim = '\0';
    *str = line;
    return delim+1;
}

static char* parseTexture(char *line, char **texture)
{
    line = parseString(line, texture);
    if(!line) return NULL;
    if(strcmp(*texture, "NULL"))
        *texture = NULL;
    return line;
}

static char* parseSide(char *line, Side *side)
{
    char *tex;
    line = parseString(line, &tex);
    if(!line) return NULL;
    if(strcmp(tex, "NULL") != 0)
        side->lowerTex = string_cstr(tex);
    line = parseString(line, &tex);
    if(!line) return NULL;
    if(strcmp(tex, "NULL") != 0)
        side->middleTex = string_cstr(tex);
    line = parseString(line, &tex);
    if(!line) return NULL;
    if(strcmp(tex, "NULL") != 0)
        side->upperTex = string_cstr(tex);
    return line;
}

bool LoadMap(Map *map)
{
    if(map->file == NULL) return false;

    FILE *file = fopen(map->file, "r");
    if(!file)
    {
        LogError("Failed to load map file %s: %s", map->file, strerror(errno));
        return false;
    }

    NewMap(map);

    map->vertexIdx = map->lineIdx = map->sectorIdx = 0;
    bool inBlock = false;
    enum ParseMode mode = PARSE_PROPS;
    char readline[1024] = { 0 };
    int lineNr = 1;
    while(fgets(readline, sizeof readline, file) != NULL)
    {
        char *line = trim(readline);
        if(!inBlock)
        {
            char *delim = strchr(line, '=');
            if(!delim)
            {
                LogError("Failed to parse map file %s: Error on line %d", map->file, lineNr);
                break;
            }

            ptrdiff_t delimPos = delim - line;
            line[delimPos] = '\0';
            char *key = trim(line);
            char *value = trim(line + delimPos + 1);

            if(mode != PARSE_PROPS)
            {
                if(strcasecmp(value, "{") == 0)
                {
                    inBlock = true;
                    continue;
                }
            }

            if(strcasecmp(key, "vertices") == 0)
            {
                mode = PARSE_VERTICES;
                inBlock = strcasecmp(value, "{") == 0;
                continue;
            }
            if(strcasecmp(key, "lines") == 0)
            {
                mode = PARSE_LINES;
                inBlock = strcasecmp(value, "{") == 0;
                continue;
            }
            if(strcasecmp(key, "sectors") == 0)
            {
                mode = PARSE_SECTORS;
                inBlock = strcasecmp(value, "{") == 0;
                continue;
            }

            if(strcasecmp(key, "version") == 0)
            {
                char *end;
                errno = 0;
                int version = strtol(value, &end, 10);
                if(end == value)
                {
                    LogError("Failed to parse the version: %s", strerror(errno));
                    break;
                }
                else
                {
                    if(version > MAP_VERSION)
                    {
                        LogError("Map format version too new (%d > %d)", version, MAP_VERSION);
                        break;
                    }
                }
            }

            if(strcasecmp(key, "gravity") == 0)
            {
                errno = 0;
                char *end;
                float gravity = strtof(value, &end);
                if(end == value)
                {
                    LogWarning("Failed to parse the gravity: %s", strerror(errno));
                    LogWarning("Using default gravity");
                    gravity = 9.8f;
                }
                map->gravity = gravity;
            }

            if(strcasecmp(key, "textureScale") == 0)
            {
                errno = 0;
                char *end;
                int textureScale = strtol(value, &end, 10);
                if(end == value)
                {
                    LogWarning("Failed to parse the textureScale: %s", strerror(errno));
                    LogWarning("Using default textureScale");
                    textureScale = 1;
                }
                map->textureScale = textureScale;
            }
        }
        else
        {
            if(strcasecmp(line, "}") == 0)
            {
                inBlock = false;
                mode = PARSE_PROPS;
                continue;
            }

            switch(mode)
            {
            case PARSE_VERTICES:
                {
                    size_t idx;
                    line = parseIndex(line, &idx);
                    if(!line) continue;
                    vec2s pos;
                    line = parseFloat(line, &pos.x);
                    if(!line) continue;
                    line = parseFloat(line, &pos.y);

                    CreateResult res = CreateVertex(map, pos);
                    MapVertex *vertex = res.mapElement;
                    vertex->idx = idx;

                    if(idx > map->vertexIdx) map->vertexIdx = idx;
                }
                break;
            case PARSE_LINES:
                {
                    LineData data = { 0 };

                    size_t idx;
                    line = parseIndex(line, &idx);
                    if(!line) continue;
                    size_t vertexA;
                    line = parseIndex(line, &vertexA);
                    if(!line) continue;
                    size_t vertexB;
                    line = parseIndex(line, &vertexB);
                    if(!line) continue;
                    line = parseUint(line, &data.type);
                    if(!line) continue;
                    line = parseSide(line, &data.front);
                    if(!line) continue;
                    line = parseSide(line, &data.back);
                    //if(!line) continue;

                    MapVertex *vA = GetVertex(map, vertexA);
                    if(!vA) continue;
                    MapVertex *vB = GetVertex(map, vertexB);
                    if(!vB) continue;
                    CreateResult res = CreateLine(map, vA, vB, data);
                    MapLine *mapLine = res.mapElement;
                    mapLine->idx = idx;

                    if(idx > map->lineIdx) map->lineIdx = idx;

                    FreeLineData(data);
                }
                break;
            case PARSE_SECTORS:
                {
                    SectorData data = { 0 };

                    size_t idx;
                    line = parseIndex(line, &idx);
                    if(!line) continue;
                    size_t numOuterLines;
                    line = parseIndex(line, &numOuterLines);
                    if(!line) continue;

                    MapLine *outerLines[numOuterLines];
                    for(size_t i = 0; i < numOuterLines; ++i)
                    {
                        size_t lineIdx;
                        line = parseIndex(line, &lineIdx);
                        if(!line) goto nextLine;
                        MapLine *mapLine = GetLine(map, lineIdx);
                        if(!mapLine) goto nextLine;
                        outerLines[i] = mapLine;
                    }

                    line = parseInt(line, &data.floorHeight);
                    if(!line) continue;
                    line = parseInt(line, &data.ceilHeight);
                    if(!line) continue;
                    line = parseUint(line, &data.type);

                    char *floorTexture;
                    line = parseTexture(line, &floorTexture);
                    if(!line) continue;
                    if(floorTexture)
                        data.floorTex = string_cstr(floorTexture);
                    char *ceilTexture;
                    line = parseTexture(line, &ceilTexture);
                    if(!line) continue;
                    if(ceilTexture)
                        data.ceilTex = string_cstr(ceilTexture);

                    CreateResult res = CreateSector(map, numOuterLines, outerLines, NULL, data);
                    MapSector *sector = res.mapElement;
                    sector->idx = idx;

                    if(idx > map->lineIdx) map->lineIdx = idx;

                    FreeSectorData(data);
                }
                break;
            default:
                break;
            }
nextLine:
        }

        lineNr++;
    }

    fclose(file);
    map->dirty = false;
    return false;
}

static char* getTextureName(pstring texname)
{
    return texname ? texname : "NULL";
}

void SaveMap(Map *map)
{
    if(map->file == NULL) return;

    FILE *file = fopen(map->file, "w");
    if(!file)
    {
        LogError("Failed to create map file %s: %s", map->file, strerror(errno));
        return;
    }

    fprintf(file, "version = %d\n", MAP_VERSION);
    fprintf(file, "editor = editor2\n");
    fprintf(file, "gravity = %.4f\n", map->gravity);
    fprintf(file, "textureScale = %d\n", map->textureScale);

    fprintf(file, "vertices = {\n");
    for(MapVertex *vertex = map->headVertex; vertex; vertex = vertex->next)
    {
        fprintf(file, "\t%zu %.4f %.4f\n", vertex->idx, vertex->pos.x, vertex->pos.y);
    }
    fprintf(file, "}\n");

    fprintf(file, "lines = {\n");
    for(MapLine *line = map->headLine; line; line = line->next)
    {
        fprintf(file, "\t%zu %zu %zu %u ", line->idx, line->a->idx, line->b->idx, line->data.type);
        fprintf(file, "%s %s %s ", getTextureName(line->data.front.lowerTex), getTextureName(line->data.front.middleTex), getTextureName(line->data.front.upperTex));
        fprintf(file, "%s %s %s\n", getTextureName(line->data.back.lowerTex), getTextureName(line->data.back.middleTex), getTextureName(line->data.back.upperTex));
    }
    fprintf(file, "}\n");

    fprintf(file, "sectors = {\n");
    for(MapSector *sector = map->headSector; sector; sector = sector->next)
    {
        fprintf(file, "\t%zu %zu ", sector->idx, sector->numOuterLines);
        for(size_t i = 0; i < sector->numOuterLines; ++i)
        {
            fprintf(file, "%zu ", sector->outerLines[i]->idx);
        }
        fprintf(file, "%d %d %u ", sector->data.floorHeight, sector->data.ceilHeight, sector->data.type);
        fprintf(file, "%s %s\n", getTextureName(sector->data.floorTex), getTextureName(sector->data.ceilTex));
    }
    fprintf(file, "}\n");

    fclose(file);
    map->dirty = false;
}

void FreeMap(Map *map)
{
    FreeVertList(map->headVertex);
    FreeLineList(map->headLine);
    FreeSectorList(map->headSector);

    string_free(map->file);
    map->file = NULL;
}

