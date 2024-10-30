#include "../dialogs.h"
#include "ImGuiFileDialog.h"
#include "utils/string.h"

#include "memory.h" // IWYU pragma: keep

static void OpenMapCallback(const char *path, void *data)
{
    Map *map = data;
    free(map->file);
    map->file = CopyString(path);
    LoadMap(map);
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
