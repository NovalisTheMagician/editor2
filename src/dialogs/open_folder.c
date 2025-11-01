#include "../dialogs.h"
#include "ImGuiFileDialog.h"

#include "../project.h"

#include <string.h>
#include <stdlib.h>

static void OpenFolderCallback(const char *path, void *data)
{
    strncpy(data, path, MAX_ASSETPATH_LEN);
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
