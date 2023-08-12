#include "../dialogs.h"

static void OpenMapCallback(const char *path, void *data)
{
    struct Map *map = data;
    pstr_free(map->file);
    map->file = pstr_cstr(path);
    LoadMap(map);
}

void OpenMapDialog(struct Map *map)
{
    struct FileDialogAction *fda = calloc(1, sizeof *fda);
    fda->data = map;
    fda->callback = OpenMapCallback;
    IGFD_OpenDialog(cfileDialog, "filedlg", "Open Map", "Map Files(*.map){.map}, All(*.*){.*}", ".", "", 1, fda, ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_ReadOnlyFileNameField | ImGuiFileDialogFlags_CaseInsensitiveExtention);
}
