#include "project.h"

#include "memory.h" // IWYU pragma: keep

#include <stdio.h>
#include <string.h>

#include "logging.h"
#include "serialization.h"

#define PROJECT_VERSION 2

void NewProject(Project *project)
{
    free(project->file);
    project->file = NULL;

    memset(project->basePath.ftp.path, 0, sizeof project->basePath.ftp.path);
    memset(project->basePath.ftp.url, 0, sizeof project->basePath.ftp.url);
    memset(project->basePath.ftp.login, 0, sizeof project->basePath.ftp.login);
    memset(project->basePath.ftp.password, 0, sizeof project->basePath.ftp.password);

    strncpy(project->texturesPath, "textures", sizeof project->texturesPath);
    strncpy(project->thingsFile, "things.txt", sizeof project->thingsFile);

    project->dirty = false;
}

bool LoadProject(Project *project)
{
    if(!project->file) return false;
    FILE *file = fopen(project->file, "r");
    if(!file)
    {
        LogError("Failed to load project file %s: %s", project->file, strerror(errno));
        return false;
    }

    char lineRaw[1024] = { 0 };
    int version = -1;
    int lineNr = 0;
    while(fgets(lineRaw, sizeof lineRaw, file) != NULL)
    {
        lineNr++;
        char *line = Trim(lineRaw);
        if(line[0] == '/' && line[1] == '/') continue; // comment
        char *delim = strchr(line, '=');
        if(!delim || delim == line)
        {
            LogError("Malformed line at %d", lineNr);
            goto errorParse;
        }
        *delim = '\0';
        char *key = line;
        char *value = delim+1;

        if(strcasecmp(key, "version") == 0)
        {
            if(ParseInt(value, &version))
            {
                if(version != PROJECT_VERSION)
                {
                    LogError("Unsupported project version on line %d (got %d | expected %d)", lineNr, version, PROJECT_VERSION);
                    goto errorParse;
                }
                continue;
            }
        }
        if(strcasecmp(key, "textures_path") == 0) { strncpy(project->texturesPath, value, sizeof project->texturesPath); continue; }
        if(strcasecmp(key, "things_file") == 0) { strncpy(project->thingsFile, value, sizeof project->thingsFile); continue; }
        if(strcasecmp(key, "source_type") == 0) { if(ParseUint(value, &project->basePath.type)) continue; }
        if(strcasecmp(key, "source_path") == 0) { strncpy(project->basePath.ftp.path, value, sizeof project->basePath.ftp.path); continue; }
        if(strcasecmp(key, "source_url") == 0) { strncpy(project->basePath.ftp.url, value, sizeof project->basePath.ftp.url); continue; }
        if(strcasecmp(key, "source_login") == 0) { strncpy(project->basePath.ftp.login, value, sizeof project->basePath.ftp.login); continue; }
        if(strcasecmp(key, "source_password") == 0) { strncpy(project->basePath.ftp.password, value, sizeof project->basePath.ftp.password); continue; }

        LogError("Failed to parse line %d", lineNr);
        goto errorParse;
    }

    if(version == -1)
    {
        LogError("No version key found");
        goto errorParse;
    }

    fclose(file);
    project->dirty = false;
    return true;

errorParse:
    fclose(file);
    LogError("Failed to load project file %s", project->file);
    NewProject(project);
    return false;
}

void SaveProject(Project *project)
{
    if(!project->file) return;
    FILE *file = fopen(project->file, "w");
    if(!file)
    {
        LogError("Failed to save project file %s: %s", project->file, strerror(errno));
        return;
    }

    fprintf(file, "version = %d\n", PROJECT_VERSION);
    fprintf(file, "textures_path = %s\n", project->texturesPath);
    fprintf(file, "things_file = %s\n", project->thingsFile);
    fprintf(file, "source_type = %d\n", project->basePath.type);
    fprintf(file, "\n");
    if(project->basePath.type == ASSPATH_FS)
    {
        fprintf(file, "source_path = %s\n", project->basePath.fs.path);
    }
    else
    {
        fprintf(file, "source_url = %s\n", project->basePath.ftp.url);
        fprintf(file, "source_login = %s\n", project->basePath.ftp.login);
        fprintf(file, "source_password = %s\n", project->basePath.ftp.password);
        fprintf(file, "source_path = %s\n", project->basePath.ftp.path);
    }

    fclose(file);
    project->dirty = false;
}

void FreeProject(Project *project)
{
    free(project->file);
    project->file = NULL;
}
