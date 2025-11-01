#include "map.h"

#include "edit.h"
#include "logging.h"
#include "map/query.h"
#include "serialization.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <strings.h>

#define KEY_VERSION "version"
#define KEY_EDITOR "editor"
#define KEY_GRAVITY "gravity"
#define KEY_TEXTURESCALE "texture_scale"
#define KEY_VERTICES "vertices"
#define KEY_LINES "lines"
#define KEY_SECTORS "sectors"

#define KEY_IS(k) strcasecmp(key, k) == 0

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
    {
        copy.front.lowerTex = malloc(strlen(data.front.lowerTex)+1);
        strcpy(copy.front.lowerTex, data.front.lowerTex);
    }
    if(data.front.middleTex)
    {
        copy.front.middleTex = malloc(strlen(data.front.middleTex)+1);
        strcpy(copy.front.middleTex, data.front.middleTex);
    }
    if(data.front.upperTex)
    {
        copy.front.upperTex = malloc(strlen(data.front.upperTex)+1);
        strcpy(copy.front.upperTex, data.front.upperTex);
    }
    if(data.back.lowerTex)
    {
        copy.back.lowerTex = malloc(strlen(data.back.lowerTex)+1);
        strcpy(copy.back.lowerTex, data.back.lowerTex);
    }
    if(data.back.middleTex)
    {
        copy.back.middleTex = malloc(strlen(data.back.middleTex)+1);
        strcpy(copy.back.middleTex, data.back.middleTex);
    }
    if(data.back.upperTex)
    {
        copy.back.upperTex = malloc(strlen(data.back.upperTex)+1);
        strcpy(copy.back.upperTex, data.back.upperTex);
    }
    return copy;
}

SectorData CopySectorData(SectorData data)
{
    SectorData copy = data;
    if(data.ceilTex)
    {
        copy.ceilTex = malloc(strlen(data.ceilTex)+1);
        strcpy(copy.ceilTex, data.ceilTex);
    }
    if(data.floorTex)
    {
        copy.floorTex = malloc(strlen(data.floorTex)+1);
        strcpy(copy.floorTex, data.floorTex);
    }
    return copy;
}

void FreeLineData(LineData data)
{
    free(data.front.lowerTex);
    free(data.front.middleTex);
    free(data.front.upperTex);
    free(data.back.lowerTex);
    free(data.back.middleTex);
    free(data.back.upperTex);
}

void FreeSectorData(SectorData data)
{
    free(data.ceilTex);
    free(data.floorTex);
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

    free(map->file);
    map->file = NULL;

    map->dirty = false;

    map->textureScale = 1;
    map->gravity = 9.80f;
}

enum ParseMode
{
    PARSE_PROPS,
    PARSE_VERTICES,
    PARSE_LINES,
    PARSE_SECTORS
};

static char* parseSide(char *line, Side *side)
{
    char *tex;
    line = ParseLineString(line, &tex);
    if(!line) return NULL;
    if(strcmp(tex, "NULL") != 0)
    {
        side->lowerTex = malloc(strlen(tex)+1);
        strcpy(side->lowerTex, tex);
    }
    line = ParseLineString(line, &tex);
    if(!line) return NULL;
    if(strcmp(tex, "NULL") != 0)
    {
        side->middleTex = malloc(strlen(tex)+1);
        strcpy(side->middleTex, tex);
    }
    line = ParseLineString(line, &tex);
    if(!line) return NULL;
    if(strcmp(tex, "NULL") != 0)
    {
        side->upperTex = malloc(strlen(tex)+1);
        strcpy(side->upperTex, tex);
    }
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
    while(fgets(readline, sizeof readline, file))
    {
        char *line = Trim(readline);
        if(!inBlock)
        {
            char *delim = strchr(line, '=');
            if(!delim || delim == line)
            {
                LogError("Failed to parse map file %s: Error on line %d", map->file, lineNr);
                break;
            }

            *delim = '\0';
            char *key = Trim(line);
            char *value = Trim(delim + 1);

            if(mode != PARSE_PROPS)
            {
                if(strcmp(value, "{") == 0)
                {
                    inBlock = true;
                    continue;
                }
            }

            if(KEY_IS(KEY_VERTICES))
            {
                mode = PARSE_VERTICES;
                inBlock = strcmp(value, "{") == 0;
                continue;
            }
            if(KEY_IS(KEY_LINES))
            {
                mode = PARSE_LINES;
                inBlock = strcmp(value, "{") == 0;
                continue;
            }
            if(KEY_IS(KEY_SECTORS))
            {
                mode = PARSE_SECTORS;
                inBlock = strcmp(value, "{") == 0;
                continue;
            }

            if(KEY_IS(KEY_VERSION))
            {
                int version;
                if(!ParseInt(value, &version))
                {
                    LogError("Failed to parse the version");
                    break;
                }
                else if(version > MAP_VERSION)
                {
                    LogError("Map format version too new (%d > %d)", version, MAP_VERSION);
                    break;
                }
            }

            if(KEY_IS(KEY_GRAVITY))
            {
                if(!ParseFloat(value, &map->gravity))
                {
                    LogWarning("Failed to parse the gravity");
                    LogWarning("Using default gravity");
                    map->gravity = 9.8f;
                }
            }

            if(KEY_IS(KEY_TEXTURESCALE))
            {
                if(ParseInt(value, &map->textureScale))
                {
                    LogWarning("Failed to parse the textureScale");
                    LogWarning("Using default textureScale");
                    map->textureScale = 1;
                }
            }
        }
        else
        {
            if(strcmp(line, "}") == 0)
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
                    line = ParseLineIndex(line, &idx);
                    if(!line) continue;
                    Vec2 pos;
                    line = ParseLineReal(line, &pos.x);
                    if(!line) continue;
                    line = ParseLineReal(line, &pos.y);

                    MapVertex *vertex = EditAddVertex(map, pos);;
                    vertex->idx = idx;

                    if(idx > map->vertexIdx) map->vertexIdx = idx;
                }
                break;
            case PARSE_LINES:
                {
                    LineData data = { 0 };

                    size_t idx;
                    line = ParseLineIndex(line, &idx);
                    if(!line) continue;
                    size_t vertexA;
                    line = ParseLineIndex(line, &vertexA);
                    if(!line) continue;
                    size_t vertexB;
                    line = ParseLineIndex(line, &vertexB);
                    if(!line) continue;
                    line = ParseLineUint(line, &data.type);
                    if(!line) continue;
                    line = parseSide(line, &data.front);
                    if(!line) continue;
                    line = parseSide(line, &data.back);

                    MapVertex *vA = GetVertex(map, vertexA);
                    if(!vA) continue;
                    MapVertex *vB = GetVertex(map, vertexB);
                    if(!vB) continue;
                    MapLine *mapLine = EditAddLine(map, vA, vB, data);
                    mapLine->idx = idx;

                    if(idx > map->lineIdx) map->lineIdx = idx;

                    FreeLineData(data);
                }
                break;
            case PARSE_SECTORS:
                {
                    SectorData data = { 0 };

                    size_t idx;
                    line = ParseLineIndex(line, &idx);
                    if(!line) continue;
                    size_t numOuterLines;
                    line = ParseLineIndex(line, &numOuterLines);
                    if(!line) continue;

                    MapLine *outerLines[numOuterLines];
                    for(size_t i = 0; i < numOuterLines; ++i)
                    {
                        size_t lineIdx;
                        line = ParseLineIndex(line, &lineIdx);
                        if(!line) goto nextLine;
                        MapLine *mapLine = GetLine(map, lineIdx);
                        if(!mapLine) goto nextLine;
                        outerLines[i] = mapLine;
                    }

                    line = ParseLineInt(line, &data.floorHeight);
                    if(!line) continue;
                    line = ParseLineInt(line, &data.ceilHeight);
                    if(!line) continue;
                    line = ParseLineUint(line, &data.type);

                    char *floorTexture;
                    line = ParseLineTexture(line, &floorTexture);
                    if(!line) continue;
                    if(floorTexture)
                    {
                        data.floorTex = malloc(strlen(floorTexture)+1);
                        strcpy(data.floorTex, floorTexture);
                    }
                    char *ceilTexture;
                    line = ParseLineTexture(line, &ceilTexture);
                    if(ceilTexture)
                    {
                        data.ceilTex = malloc(strlen(ceilTexture)+1);
                        strcpy(data.ceilTex, ceilTexture);
                    }

                    MapSector *sector = EditAddSector(map, numOuterLines, outerLines, 0, (size_t[0]){}, (MapLine**[0]){}, data);
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
    return true;
}

static char* getTextureName(char *texname)
{
    return texname ? texname : "NULL";
}

void SaveMap(Map *map)
{
    if(!map->file) return;

    FILE *file = fopen(map->file, "w");
    if(!file)
    {
        LogError("Failed to save map file %s: %s", map->file, strerror(errno));
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

    free(map->file);
    map->file = NULL;
}

