#include "texture_load_ftp.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "../logging.h"
#include "../utils/string.h"

extern pthread_mutex_t collectMutex;
extern bool cancelRequest;

netbuf* ConnectToFtp(const char *url, const char *user, const char *pass)
{
    netbuf *handle;
    if(!FtpConnect(url, &handle))
    {
        LogError("failed to connect to ftp server: %s", url);
        return NULL;
    }

    if(!FtpLogin(user, pass, handle))
    {
        LogError("failed to login to ftp server: %s", user);
        FtpQuit(handle);
        return NULL;
    }

    return handle;
}

size_t CollectTexturesFtp(TextureCollection *tc, FetchLocation **locations, size_t size, size_t *capacity, char *folder, char *baseFolder, netbuf *ftpHandle)
{
    netbuf *dirHandle;
    if(!FtpAccess(folder, FTPLIB_DIR_VERBOSE, FTPLIB_ASCII, ftpHandle, &dirHandle))
    {
        LogError("%s:", FtpLastResponse(ftpHandle));
        LogError("failed to get dir listing: %s", folder);
        return 0;
    }

    size_t cap = 4096;
    struct
    {
        char *filePath;
        char *fileName;
        bool isDir;
    } *files = calloc(cap, sizeof *files);

    bool stopRequest = false;

    char buffer[1024] = { 0 };
    size_t numFiles = 0;
    ssize_t readLen;
    while((readLen = FtpRead(buffer, sizeof buffer, dirHandle)) > 0)
    {
        pthread_mutex_lock(&collectMutex);
        stopRequest = cancelRequest;
        pthread_mutex_unlock(&collectMutex);
        if(stopRequest) break;

        char *delim = strchr(buffer, ' ');
        *delim = '\0';
        char *perm = buffer;
        bool isDir = perm[0] == 'd';
        char *fn = NULL;
        while((delim = strchr(delim+1, ' ')) != NULL) fn = delim+1;
        if(fn == NULL)
        {
            LogWarning("failed to parse the filename out of a directory listing `%s`", buffer);
            continue;
        }

        size_t fnLen = strlen(fn);
        if(fn[fnLen-1] == '\n') fn[fnLen-1] = '\0';
        char *fileName = fn;
        char filePath[256] = { 0 };
        snprintf(filePath, sizeof filePath, "%s/%s", folder, fileName);

        files[numFiles++] = (typeof(*files)){ .fileName = CopyString(fileName), .filePath = CopyString(filePath), .isDir = isDir };
    }
    FtpClose(dirHandle);

    for(size_t i = 0; i < numFiles && !stopRequest; ++i)
    {
        pthread_mutex_lock(&collectMutex);
        stopRequest = cancelRequest;
        pthread_mutex_unlock(&collectMutex);

        if(files[i].isDir)
        {
            size = CollectTexturesFtp(tc, locations, size, capacity, files[i].filePath, baseFolder, ftpHandle);
            if(size >= *capacity)
            {
                size_t oldCapacity = *capacity;
                *capacity = (*capacity) * 2;
                *locations = realloc(*locations, (*capacity) * sizeof **locations);
                memset((*locations) + oldCapacity, 0, (*capacity) - oldCapacity);
            }
        }
        else
        {
            char *path = CopyString(NormalizePath(files[i].filePath));
            char *ext = strrchr(path, '.');
            char *name = strstr(path, baseFolder);
            if(!name) goto skipLocation;
            if(!ext) goto skipLocation;
            name += strlen(baseFolder);
            if(name[0] == '/') name++;
            *ext = '\0';

            char timeBuffer[128] = { 0 };
            if(!FtpModDate(files[i].filePath, timeBuffer, sizeof timeBuffer, ftpHandle))
            {
                LogError("%s", FtpLastResponse(ftpHandle));
                goto skipLocation;
            }

            struct tm tm = { .tm_isdst = -1 };
            // 'YYYYMMDDHHMMSS'
#if defined(_WIN32)
            sscanf(timeBuffer, "%4d%2d%2d%2d%2d%2d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
#else
            strptime(timeBuffer, "%Y%m%d%H%M%S", &tm);
#endif
            time_t timestamp = mktime(&tm);

            if(!tc_is_newer(tc, name, timestamp))
            {
                goto skipLocation;
            }

            size_t idx = size++;
            (*locations)[idx] = (FetchLocation){ .name = CopyString(name), .path = CopyString(files[i].filePath), .mtime = timestamp };

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

    for(size_t i = 0; i < numFiles; ++i)
    {
        free(files[i].filePath);
        free(files[i].fileName);
    }
    free(files);
    return size;
}

bool ReadFromFtp(const char *path, uint8_t **buffer, size_t *size, void *ftpHandle)
{
    uint32_t fileSize = 0;
    if(!FtpSize(path, &fileSize, FTPLIB_BINARY, ftpHandle))
    {
        LogError("%s", FtpLastResponse(ftpHandle));
        return false;
    }
    *size = fileSize;

    netbuf *fileHandle;
    if(!FtpAccess(path, FTPLIB_FILE_READ, FTPLIB_BINARY, ftpHandle, &fileHandle))
    {
        LogError("%s", FtpLastResponse(ftpHandle));
        return false;
    }

    *buffer = calloc(*size, sizeof **buffer);
    size_t readTotal = 0, readCurrent;
    while((readCurrent = FtpRead((*buffer) + readTotal, (*size) - readTotal, fileHandle)) > 0)
    {
        readTotal += readCurrent;
    }

    FtpClose(fileHandle);
    return true;
}
