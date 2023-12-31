#pragma once

#include "editor.h"
#include "common.h"
#include <ImGuiFileDialog.h>

enum SaveModalAction
{
    SMA_NEW,
    SMA_OPEN,
    SMA_QUIT
};

struct FileDialogAction 
{
    void *data;
    void (*callback)(const char *path, void *data);
    bool quitRequest;
};

extern ImGuiFileDialog *cfileDialog;

void OpenFolderDialog(pstring *folderPath);
void SaveMapDialog(struct Map *map, bool quitRequest);
void SaveProjectDialog(struct Project *project, bool quitRequest);
void OpenMapDialog(struct Map *map);
void OpenProjectDialog(struct Project *project);
