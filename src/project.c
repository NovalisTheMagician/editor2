#include "project.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "logging.h"
#include "serialization.h"

#define PROJECT_VERSION 2

#define KEY_VERSION "version"
#define KEY_TEXTURESPATH "textures_path"
#define KEY_THINGSFILE "things_file"
#define KEY_SOURCETYPE "source_type"
#define KEY_SOURCEPATH "source_path"
#define KEY_SOURCELOGIN "source_login"
#define KEY_SOURCEPASS "source_password"
#define KEY_SOURCEURL "source_url"

#define KEY_IS(k) strcasecmp(key, k) == 0

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
        if(!delim) continue;
        if(delim == line)
        {
            LogError("Malformed line at %d", lineNr);
            goto errorParse;
        }
        *delim = '\0';
        char *key = Trim(line);
        char *value = Trim(delim+1);

        if(KEY_IS(KEY_VERSION))
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
        if(KEY_IS(KEY_TEXTURESPATH)) { strncpy(project->texturesPath, value, sizeof project->texturesPath); continue; }
        if(KEY_IS(KEY_THINGSFILE)) { strncpy(project->thingsFile, value, sizeof project->thingsFile); continue; }
        if(KEY_IS(KEY_SOURCETYPE)) { if(ParseUint(value, &project->basePath.type)) continue; }
        if(KEY_IS(KEY_SOURCEPATH)) { strncpy(project->basePath.ftp.path, value, sizeof project->basePath.ftp.path); continue; }
        if(KEY_IS(KEY_SOURCEURL)) { strncpy(project->basePath.ftp.url, value, sizeof project->basePath.ftp.url); continue; }
        if(KEY_IS(KEY_SOURCELOGIN)) { strncpy(project->basePath.ftp.login, value, sizeof project->basePath.ftp.login); continue; }
        if(KEY_IS(KEY_SOURCEPASS)) { strncpy(project->basePath.ftp.password, value, sizeof project->basePath.ftp.password); continue; }

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

    fprintf(file, KEY_VERSION" = %d\n", PROJECT_VERSION);
    fprintf(file, KEY_TEXTURESPATH" = %s\n", project->texturesPath);
    fprintf(file, KEY_THINGSFILE" = %s\n", project->thingsFile);
    fprintf(file, KEY_SOURCETYPE" = %d\n", project->basePath.type);
    fprintf(file, "\n");
    if(project->basePath.type == ASSPATH_FS)
    {
        fprintf(file, KEY_SOURCEPATH" = %s\n", project->basePath.fs.path);
    }
    else
    {
        fprintf(file, KEY_SOURCEURL" = %s\n", project->basePath.ftp.url);
        fprintf(file, KEY_SOURCELOGIN" = %s\n", project->basePath.ftp.login);
        fprintf(file, KEY_SOURCEPASS" = %s\n", project->basePath.ftp.password);
        fprintf(file, KEY_SOURCEPATH" = %s\n", project->basePath.ftp.path);
    }

    fclose(file);
    project->dirty = false;
}

void FreeProject(Project *project)
{
    free(project->file);
    project->file = NULL;
}
