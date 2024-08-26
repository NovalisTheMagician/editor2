#include "../dialogs.h"

static void SaveMapCallback(const char *path, void *data)
{
    struct Map *map = data;
    string_free(map->file);
    map->file = string_cstr(path);
    SaveMap(map);
}

void SaveMapDialog(struct Map *map, bool quitRequest)
{
    struct FileDialogAction *fda = calloc(1, sizeof *fda);
    fda->data = map;
    fda->callback = SaveMapCallback;
    fda->quitRequest = quitRequest;
    struct IGFD_FileDialog_Config config = 
    {
        .flags = ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_ConfirmOverwrite | ImGuiFileDialogFlags_CaseInsensitiveExtentionFiltering,
        .path = ".",
        .filePathName = "",
        .fileName = "",
        .countSelectionMax = 1,
        .userDatas = fda,
    };
    IGFD_OpenDialog(cfileDialog, "filedlg", "Save Map", "Map Files(*.map){.map}", config);
}
