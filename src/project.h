#pragma once

#include <stdint.h>
#include <stddef.h>
#include "utils/pstring.h"

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
            pstring path;
        } fs;

        struct
        {
            pstring path;
            pstring url;
            pstring login;
            pstring password;
        } ftp;
    };
} AssetPath;

typedef struct Project
{
    AssetPath basePath;
    pstring texturesPath;
    pstring thingsFile;
    bool dirty;
    pstring file;
} Project;

void NewProject(Project *project);
bool LoadProject(Project *project);
void SaveProject(Project *project);
void FreeProject(Project *project);
