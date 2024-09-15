#include "../dialogs.h"
#include "ImGuiFileDialog.h"

static void SaveProjectCallback(const char *path, void *data)
{
    Project *project = data;
    string_free(project->file);
    project->file = string_cstr(path);
    SaveProject(project);
}

void SaveProjectDialog(Project *project, bool quitRequest)
{
    FileDialogAction *fda = calloc(1, sizeof *fda);
    fda->data = project;
    fda->callback = SaveProjectCallback;
    fda->quitRequest = quitRequest;
    struct IGFD_FileDialog_Config config = IGFD_FileDialog_Config_Get();
    config.flags = ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_ConfirmOverwrite | ImGuiFileDialogFlags_CaseInsensitiveExtentionFiltering;
    config.path = ".";
    config.userDatas = fda;
    IGFD_OpenDialog(cfileDialog, "filedlg", "Save Project", "Project Files(*.epr){.epr}", config);
}
