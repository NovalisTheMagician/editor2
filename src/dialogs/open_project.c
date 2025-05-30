#include "../dialogs.h"
#include "ImGuiFileDialog.h"
#include "utils/string.h"

static void OpenProjectCallback(const char *path, void *data)
{
    Project *project = data;
    free(project->file);
    project->file = CopyString(path);
    LoadProject(project);
}

void OpenProjectDialog(Project *project)
{
    FileDialogAction *fda = calloc(1, sizeof *fda);
    fda->data = project;
    fda->callback = OpenProjectCallback;
    struct IGFD_FileDialog_Config config = IGFD_FileDialog_Config_Get();
    config.flags = ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_ReadOnlyFileNameField | ImGuiFileDialogFlags_CaseInsensitiveExtentionFiltering;
    config.path = ".";
    config.userDatas = fda;
    IGFD_OpenDialog(cfileDialog, "filedlg", "Open Map", "Project Files(*.epr){.epr}, All(*.*){.*}", config);
}
