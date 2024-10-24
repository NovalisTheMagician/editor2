#include "../dialogs.h"
#include "ImGuiFileDialog.h"

#include "../utils.h"

#include <string.h>

static void OpenFolderCallback(const char *path, void *data)
{
    char *str = data;
    size_t pathLen = strlen(path)+1;
    size_t dataLen = strlen(str)+1;
    size_t len = min(pathLen, dataLen);
    memset(str, 0, dataLen);
    memcpy(str, path, len);
}

void OpenFolderDialog(char *folderPath)
{
    FileDialogAction *fda = calloc(1, sizeof *fda);
    fda->data = folderPath;
    fda->callback = OpenFolderCallback;
    struct IGFD_FileDialog_Config config = IGFD_FileDialog_Config_Get();
    config.flags = ImGuiFileDialogFlags_Modal;
    config.path = ".";
    config.userDatas = fda;
    IGFD_OpenDialog(cfileDialog, "filedlg", "Choose a Folder", NULL, config);
}
