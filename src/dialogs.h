#pragma once

#include "editor.h"
#include "ImGuiFileDialog.h"

typedef enum SaveModalAction
{
    SMA_NEW,
    SMA_OPEN,
    SMA_QUIT
} SaveModalAction;

typedef struct FileDialogAction
{
    void *data;
    void (*callback)(const char *path, void *data);
    bool quitRequest;
} FileDialogAction;

extern ImGuiFileDialog *cfileDialog;

void OpenFolderDialog(pstring *folderPath);
void SaveMapDialog(Map *map, bool quitRequest);
void SaveProjectDialog(Project *project, bool quitRequest);
void OpenMapDialog(Map *map);
void OpenProjectDialog(Project *project);
