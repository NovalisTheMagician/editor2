#include "../dialogs.h"
#include "ImGuiFileDialog.h"

#include "editor.h"

#include <stdlib.h>

static void OpenMapCallback(const char *path, void *data)
{
    Map *map = data;
    //free(map->file);
    LoadMap(map, path);
    UpdateTitle(map->file);
}

void OpenMapDialog(Map *map)
{
    FileDialogAction *fda = calloc(1, sizeof *fda);
    fda->data = map;
    fda->callback = OpenMapCallback;
    struct IGFD_FileDialog_Config config = IGFD_FileDialog_Config_Get();
    config.flags = ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_ReadOnlyFileNameField | ImGuiFileDialogFlags_CaseInsensitiveExtentionFiltering;
    config.path = ".";
    config.userDatas = fda;
    IGFD_OpenDialog(cfileDialog, "filedlg", "Open Map", "Map Files(*.map){.map}, All(*.*){.*}", config);
}
