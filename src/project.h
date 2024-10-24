#pragma once

#include <stdint.h>

#define MAX_ASSETPATH_LEN 256

typedef enum AssetPathType
{
    ASSPATH_FS,
    ASSPATH_FTP
} AssetPathType;

typedef struct AssetPath
{
    uint32_t type;
    union
    {
        struct
        {
            char path[MAX_ASSETPATH_LEN];
        } fs;

        struct
        {
            char path[MAX_ASSETPATH_LEN];
            char url[MAX_ASSETPATH_LEN];
            char login[MAX_ASSETPATH_LEN];
            char password[MAX_ASSETPATH_LEN];
        } ftp;
    };
} AssetPath;

typedef struct Project
{
    AssetPath basePath;
    char texturesPath[MAX_ASSETPATH_LEN];
    char thingsFile[MAX_ASSETPATH_LEN];
    bool dirty;
    char *file;
} Project;

void NewProject(Project *project);
bool LoadProject(Project *project);
void SaveProject(Project *project);
void FreeProject(Project *project);
