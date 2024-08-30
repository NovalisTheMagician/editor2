#include "../dialogs.h"

static void OpenMapCallback(const char *path, void *data)
{
    Map *map = data;
    string_free(map->file);
    map->file = string_cstr(path);
    LoadMap(map);
}

void OpenMapDialog(Map *map)
{
    FileDialogAction *fda = calloc(1, sizeof *fda);
    fda->data = map;
    fda->callback = OpenMapCallback;
    struct IGFD_FileDialog_Config config =
    {
        .flags = ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_ReadOnlyFileNameField | ImGuiFileDialogFlags_CaseInsensitiveExtentionFiltering,
        .path = ".",
        .filePathName = "",
        .fileName = "",
        .countSelectionMax = 1,
        .userDatas = fda,
    };
    IGFD_OpenDialog(cfileDialog, "filedlg", "Open Map", "Map Files(*.map){.map}, All(*.*){.*}", config);
}
