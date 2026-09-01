#include "mapfile.h"

#include "stdio.h"

const char* getSeverity(int severity)
{
    switch(severity)
    {
    case MS_INFO: return "[INFO]";
    case MS_WARNING: return "[WARN]";
    case MS_ERROR: return "[ERRO]";
    dafault: return "[UKNW]";
    }
}

void Log(int severity, const char *msg)
{
    printf("%s %s\n", getSeverity(severity), msg);
}

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        printf("Need to supply the mapfile\n");
        return 1;
    }

    MapfileSetLogFunc(Log);

    FILE *file = fopen(argv[1], "r");
    if(!file)
    {
        perror("Failed to open file");
        return 1;
    }

    Mapfile mapfile;
    if(!MapfileParse(file, &mapfile, 0))
    {
        fclose(file);
        printf("Failed to parse mapfile %s\n", argv[1]);
        return 1;
    }

    printf("Gravity: %f\n", mapfile.gravity);
    printf("Vertices start: %zu\n", mapfile.verticesBegin);

    for(MapfileIterator iterator = MapfileVerticesIterator(&mapfile); !MapfileIteratorEnd(&iterator); MapfileIteratorNextVertex(&iterator))
    {
        MapfileVertex vertex = { 0 };
        if(MapfileIteratorGetVertex(&iterator, &vertex))
        {
            printf("Vertex %zu: (%f %f)\n", vertex.index, vertex.x, vertex.y);
        }
    }

    for(MapfileIterator iterator = MapfileLinesIterator(&mapfile); !MapfileIteratorEnd(&iterator); MapfileIteratorNextLine(&iterator))
    {
        MapfileLine line = { 0 };
        if(MapfileIteratorGetLine(&iterator, &line))
        {
            printf("Line %zu: %zu, %zu, \"%s\" (%f %f), ...\n", line.index, line.vertexA, line.vertexB, line.front.lowerTex, line.front.lowerOffsetX, line.front.lowerOffsetY);
        }
    }

    for(MapfileIterator iterator = MapfileSectorsIterator(&mapfile); !MapfileIteratorEnd(&iterator); MapfileIteratorNextSector(&iterator))
    {
        MapfileSector sector = { 0 };
        if(MapfileIteratorGetSector(&iterator, &sector))
        {
            printf("Sector %zu: %zu %s, ...\n", sector.index, sector.firstLine, sector.firstLineFront ? "true" : "false");
        }
    }

    fclose(file);
    return 0;
}

