#pragma once

#include "common.h"

#define MAX_ASSETPATH_LEN 256

enum AssetPathType
{
    ASSPATH_FS,
    ASSPATH_FTP
};

struct AssetPath
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
};

struct Project
{
    struct AssetPath basePath;
    char texturesPath[MAX_ASSETPATH_LEN];
    char thingsPath[MAX_ASSETPATH_LEN];
    bool dirty;
    char *file;
};

void NewProject(struct Project *project);
bool LoadProject(struct Project *project);
void SaveProject(struct Project *project, bool openDialog);
