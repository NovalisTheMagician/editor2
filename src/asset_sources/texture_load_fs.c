#include "texture_load_fs.h"

#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include "../logging.h"
#include "../utils/string.h"

extern pthread_mutex_t collectMutex;
extern bool cancelRequest;

size_t CollectTexturesFs(TextureCollection *tc, FetchLocation **locations, size_t size, size_t *capacity, char *folder, char *baseFolder)
{
    DIR *dp = opendir(folder);
    if(!dp)
    {
        return 0;
    }

    bool stopRequest = false;

    struct dirent *entry;
    while((entry = readdir(dp)))
    {
        pthread_mutex_lock(&collectMutex);
        stopRequest = cancelRequest;
        pthread_mutex_unlock(&collectMutex);
        if(stopRequest) break;

        char *fileName = entry->d_name;
        if(strcmp(fileName, ".") == 0 || strcmp(fileName, "..") == 0) continue;

        char filePath[512] = { 0 };
        snprintf(filePath, sizeof filePath, "%s/%s", folder, fileName);

        struct stat buf;
        int r = stat(filePath, &buf);
        if(r == -1)
        {
            LogError("failed to stat file %s: %s", filePath, strerror(errno));
            continue;
        }

        if(S_ISDIR(buf.st_mode))
        {
            size = CollectTexturesFs(tc, locations, size, capacity, filePath, baseFolder);
            if(size >= *capacity)
            {
                size_t oldCapacity = *capacity;
                *capacity = (*capacity) * 2;
                *locations = realloc(*locations, (*capacity) * sizeof **locations);
                memset((*locations) + oldCapacity, 0, (*capacity) - oldCapacity);
            }
        }
        else if(S_ISREG(buf.st_mode))
        {
            time_t timestamp = buf.st_mtime;
            char *path = CopyString(NormalizePath(filePath));
            char *ext = strrchr(path, '.');
            char *name = strstr(path, baseFolder);
            if(!name) goto skipLocation;
            if(!ext) goto skipLocation;
            name += strlen(baseFolder);
            if(name[0] == '/') name++;
            *ext = '\0';

            if(!tc_is_newer(tc, name, timestamp)) goto skipLocation;

            size_t idx = size++;
            (*locations)[idx] = (FetchLocation){ .name = CopyString(name), .path = CopyString(filePath), .mtime = timestamp };

            if(size >= *capacity)
            {
                size_t oldCapacity = *capacity;
                *capacity = (*capacity) * 2;
                *locations = realloc(*locations, (*capacity) * sizeof **locations);
                memset((*locations) + oldCapacity, 0, (*capacity) - oldCapacity);
            }
skipLocation:
            free(path);
        }
    }
    closedir(dp);

    return size;
}

bool ReadFromFs(const char *path, uint8_t **buffer, size_t *size, void *unused)
{
    (void)unused;

    FILE *file = fopen(path, "rb");
    if(!file) return false;

    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    rewind(file);

    *buffer = calloc(*size, sizeof **buffer);
    size_t readTotal = 0, readCurrent;
    while((readCurrent = fread((*buffer) + readTotal, 1, (*size) - readTotal, file)) > 0)
    {
        readTotal += readCurrent;
    }

    fclose(file);

    return true;
}
