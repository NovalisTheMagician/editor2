#include "../dialogs.h"

static void SaveProjectCallback(const char *path, void *data)
{
    struct Project *project = data;
    string_free(project->file);
    project->file = string_cstr(path);
    SaveProject(project);
}

void SaveProjectDialog(struct Project *project, bool quitRequest)
{
    struct FileDialogAction *fda = calloc(1, sizeof *fda);
    fda->data = project;
    fda->callback = SaveProjectCallback;
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
    IGFD_OpenDialog(cfileDialog, "filedlg", "Save Project", "Project Files(*.epr){.epr}", config);
}
