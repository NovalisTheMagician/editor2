#include "../dialogs.h"

static void OpenProjectCallback(const char *path, void *data)
{
    struct Project *project = data;
    string_free(project->file);
    project->file = string_cstr(path);
    LoadProject(project);
}

void OpenProjectDialog(struct Project *project)
{
    struct FileDialogAction *fda = calloc(1, sizeof *fda);
    fda->data = project;
    fda->callback = OpenProjectCallback;
    IGFD_OpenDialog(cfileDialog, "filedlg", "Open Map", "Project Files(*.pro){.pro}, All(*.*){.*}", ".", "", 1, fda, ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_ReadOnlyFileNameField | ImGuiFileDialogFlags_CaseInsensitiveExtention);
}
