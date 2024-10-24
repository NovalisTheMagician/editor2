#pragma once

#include "memory.h"
#include "map.h"
#include "project.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h" // IWYU pragma: keep
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

void OpenFolderDialog(char *folderPath);
void SaveMapDialog(Map *map, bool quitRequest);
void SaveProjectDialog(Project *project, bool quitRequest);
void OpenMapDialog(Map *map);
void OpenProjectDialog(Project *project);
