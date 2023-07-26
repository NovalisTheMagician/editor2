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
};

struct Project
{
    struct AssetPath basePath;
    pstring texturesPath;
    pstring thingsFile;
    bool dirty;
    pstring file;
};

void NewProject(struct Project *project);
bool LoadProject(struct Project *project);
void SaveProject(struct Project *project, bool openDialog);
void FreeProject(struct Project *project);
