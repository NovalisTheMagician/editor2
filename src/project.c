#include "project.h"

void NewProject(struct Project *project)
{
    if(project->file) free(project->file);
    memset(project, 0, sizeof *project);
}

bool LoadProject(struct Project *project)
{
    project->dirty = false;
    return false;
}

void SaveProject(struct Project *project, bool openDialog)
{
    project->dirty = false;
}
