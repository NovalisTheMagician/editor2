#include "../dialogs.h"
#include "ImGuiFileDialog.h"
#include "utils/string.h"

#include <stdlib.h>

static void SaveMapCallback(const char *path, void *data)
{
    Map *map = data;
    free(map->file);
    map->file = CopyString(path);
    SaveMap(map);
}

void SaveMapDialog(Map *map, bool quitRequest)
{
    FileDialogAction *fda = calloc(1, sizeof *fda);
    fda->data = map;
    fda->callback = SaveMapCallback;
    fda->quitRequest = quitRequest;
    struct IGFD_FileDialog_Config config = IGFD_FileDialog_Config_Get();
    config.flags = ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_ConfirmOverwrite | ImGuiFileDialogFlags_CaseInsensitiveExtentionFiltering;
    config.path = ".";
    config.userDatas = fda;
    IGFD_OpenDialog(cfileDialog, "filedlg", "Save Map", "Map Files(*.map){.map}", config);
}
