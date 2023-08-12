#include "../dialogs.h"

static void SaveMapCallback(const char *path, void *data)
{
    struct Map *map = data;
    pstr_free(map->file);
    map->file = pstr_cstr(path);
    SaveMap(map);
}

void SaveMapDialog(struct Map *map, bool quitRequest)
{
    struct FileDialogAction *fda = calloc(1, sizeof *fda);
    fda->data = map;
    fda->callback = SaveMapCallback;
    fda->quitRequest = quitRequest;
    IGFD_OpenDialog(cfileDialog, "filedlg", "Save Map", "Map Files(*.map){.map}", ".", "", 1, fda, ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_ConfirmOverwrite | ImGuiFileDialogFlags_CaseInsensitiveExtention);
}
