#include "project.h"

void NewProject(struct Project *project)
{
    pstr_free(project->file);
    pstr_free(project->thingsFile);
    pstr_free(project->texturesPath);
    pstr_free(project->basePath.ftp.path);
    pstr_free(project->basePath.ftp.url);
    pstr_free(project->basePath.ftp.login);
    pstr_free(project->basePath.ftp.password);

    project->basePath.ftp.path = pstr_alloc(MAX_ASSETPATH_LEN);
    project->basePath.ftp.url = pstr_alloc(MAX_ASSETPATH_LEN);
    project->basePath.ftp.login = pstr_alloc(MAX_ASSETPATH_LEN);
    project->basePath.ftp.password = pstr_alloc(MAX_ASSETPATH_LEN);

    project->file = pstr_alloc(0);
    project->texturesPath = pstr_cstr_size("textures/", MAX_ASSETPATH_LEN);
    project->thingsFile = pstr_cstr_size("things.txt", MAX_ASSETPATH_LEN);

    project->dirty = false;
}

bool LoadProject(struct Project *project)
{
    project->dirty = false;
    return false;
}

void SaveProject(struct Project *project)
{
    project->dirty = false;
}

void FreeProject(struct Project *project)
{
    pstr_free(project->file);
    pstr_free(project->thingsFile);
    pstr_free(project->texturesPath);
    pstr_free(project->basePath.ftp.path);
    pstr_free(project->basePath.ftp.url);
    pstr_free(project->basePath.ftp.login);
    pstr_free(project->basePath.ftp.password);
}
