#include "../dialogs.h"

#include <string.h>

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
    FileDialogAction *fda = calloc(1, sizeof *fda);
    fda->data = folderPath;
    fda->callback = OpenFolderCallback;
    struct IGFD_FileDialog_Config config =
    {
        .flags = ImGuiFileDialogFlags_Modal,
        .path = ".",
        .filePathName = "",
        .fileName = "",
        .countSelectionMax = 1,
        .userDatas = fda,
    };
    IGFD_OpenDialog(cfileDialog, "filedlg", "Choose a Folder", NULL, config);
}
