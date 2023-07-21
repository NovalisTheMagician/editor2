#pragma once

#include "common.h"

#define MAX_TEX_LOCATION_LEN 256

enum TexturePathType
{
    TEXPATH_FS,
    TEXPATH_FTP
};

struct TexturePath
{
    uint32_t type;
    union
    {
        struct 
        {
            char path[MAX_TEX_LOCATION_LEN];
        } fs;

        struct 
        {
            char path[MAX_TEX_LOCATION_LEN];
            char login[MAX_TEX_LOCATION_LEN];
            char password[MAX_TEX_LOCATION_LEN];
        } ftp;
    };
};

struct Project
{
    struct TexturePath texturePath;
};

void NewProject(struct Project *project);
bool LoadProject(struct Project *project);
void SaveProject(struct Project *project, bool openDialog);
