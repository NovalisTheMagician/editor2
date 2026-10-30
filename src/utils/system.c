#include "system.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

#define FOLDER_NAME "editor2"
#define FOLDER_NAME_HIDDEN ".editor2"

static char buffer[1024];

const char* combinePaths_(const char *first, ...)
{
    va_list ap;
    va_start(ap, first);

    strncpy(buffer, first, sizeof buffer - 1);
    const char* arg = NULL;
    while((arg = va_arg(ap, const char*)) != NULL)
    {
        strncat(buffer, "/", sizeof buffer - 1);
        strncat(buffer, arg, sizeof buffer - 1);
    }

    va_end(ap);
    return buffer;
}

static int checkFolder(const char *path)
{
    DIR *dir = opendir(path);
    if(dir)
    {
        closedir(dir);
        return 1;
    }
    return 0;
}

static void makeFolder(const char *path)
{
#ifdef _WIN32
    mkdir(path);
#else
    mkdir(path, 0755);
#endif
}

static const char* checkAndMakeFolder(const char *path, const char *folderName)
{
    if(path && path[0] != '\0')
    {
        if(checkFolder(path))
        {
            path = combinePaths(path, folderName);
            makeFolder(path);
            return path;
        }
    }
    return NULL;
}

const char* getPreferredFolder(const char *subpath)
{
    const char *path = NULL;
#ifndef _WIN32
    path = checkAndMakeFolder(getenv("XDG_CONFIG_HOME"), FOLDER_NAME);
    if(path) goto make_subpath;
    
    path = checkAndMakeFolder(getenv("HOME"), FOLDER_NAME_HIDDEN);
    if(path) goto make_subpath;
#endif
    if(path == NULL)
        path = ".";

make_subpath:
    path = combinePaths(path, subpath);
    makeFolder(path);

    return path;
}

