#include "../dialogs.h"

static void OpenFolderCallback(const char *path, void *data)
{
    pstring str = data;
    size_t len = strlen(path)+1;
    len = len < string_size(str) ? len : string_size(str);
    memset(str, 0, string_size(str));
    memcpy(str, path, len);
    string_recalc(str);
}

void OpenFolderDialog(pstring *folderPath)
{
    struct FileDialogAction *fda = calloc(1, sizeof *fda);
    fda->data = folderPath;
    fda->callback = OpenFolderCallback;
    IGFD_OpenDialog(cfileDialog, "filedlg", "Choose a Folder", NULL, ".", "", 1, fda, ImGuiFileDialogFlags_Modal);
}
