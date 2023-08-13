#include "../dialogs.h"

static void OpenFolderCallback(const char *path, void *data)
{
    pstring *str = data;
    size_t len = strlen(path);
    len = len < str->capacity ? len : str->capacity;
    memset(str->data, 0, str->capacity);
    memcpy(str->data, path, len);
    str->size = len;
}

void OpenFolderDialog(pstring *folderPath)
{
    struct FileDialogAction *fda = calloc(1, sizeof *fda);
    fda->data = folderPath;
    fda->callback = OpenFolderCallback;
    IGFD_OpenDialog(cfileDialog, "filedlg", "Choose a Folder", NULL, ".", "", 1, fda, ImGuiFileDialogFlags_Modal);
}
