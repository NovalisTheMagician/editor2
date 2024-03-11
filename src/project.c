#include "project.h"

#include <json-c/json.h>

#define PROJECT_VERSION 1

void NewProject(struct Project *project)
{
    string_free(project->file);
    string_free(project->thingsFile);
    string_free(project->texturesPath);
    string_free(project->basePath.ftp.path);
    string_free(project->basePath.ftp.url);
    string_free(project->basePath.ftp.login);
    string_free(project->basePath.ftp.password);

    project->basePath.ftp.path = string_alloc(MAX_ASSETPATH_LEN);
    project->basePath.ftp.url = string_alloc(MAX_ASSETPATH_LEN);
    project->basePath.ftp.login = string_alloc(MAX_ASSETPATH_LEN);
    project->basePath.ftp.password = string_alloc(MAX_ASSETPATH_LEN);

    project->file = string_alloc(1);
    project->texturesPath = string_cstr_alloc("textures", MAX_ASSETPATH_LEN);
    project->thingsFile = string_cstr_alloc("things.txt", MAX_ASSETPATH_LEN);

    project->dirty = false;
}

bool LoadProject(struct Project *project)
{
    json_object *root = json_object_from_file(project->file);
    if(!root)
    {
        return false;
    }

    bool success = true;

    json_object *versionObj, *typeObj, *texturesPathObj, *thingsObj, *baseObj;
    if(!json_object_object_get_ex(root, "version", &versionObj))
    {
        success = false;
        LogError("unknown json format");
        goto cleanup;
    }

    if(!json_object_object_get_ex(root, "type", &typeObj))
    {
        success = false;
        LogError("unknown json format");
        goto cleanup;
    }

    if(!json_object_object_get_ex(root, "textures", &texturesPathObj))
    {
        success = false;
        LogError("unknown json format");
        goto cleanup;
    }

    if(!json_object_object_get_ex(root, "things", &thingsObj))
    {
        success = false;
        LogError("unknown json format");
        goto cleanup;
    }

    if(!json_object_object_get_ex(root, "base", &baseObj))
    {
        success = false;
        LogError("unknown json format");
        goto cleanup;
    }

    int version = json_object_get_int(versionObj);
    if(version != PROJECT_VERSION)
    {
        success = false;
        LogError("project version mismatch");
        goto cleanup;
    }

    project->basePath.type = json_object_get_int(typeObj);
    if(project->basePath.type != ASSPATH_FS && project->basePath.type != ASSPATH_FTP)
    {
        success = false;
        LogError("invalid base type");
        goto cleanup;
    }

    string_copy_into_cstr(project->texturesPath, json_object_get_string(texturesPathObj));
    string_copy_into_cstr(project->thingsFile, json_object_get_string(thingsObj));

    if(project->basePath.type == ASSPATH_FS)
    {
        json_object *pathObj;
        if(!json_object_object_get_ex(baseObj, "path", &pathObj))
        {
            success = false;
            LogError("unknown json format");
            goto cleanup;
        }

        string_copy_into_cstr(project->basePath.fs.path, json_object_get_string(pathObj));
    }
    else
    {
        json_object *pathObj, *urlObj, *loginObj, *passObj;
        if(!json_object_object_get_ex(baseObj, "path", &pathObj))
        {
            success = false;
            LogError("unknown json format");
            goto cleanup;
        }

        if(!json_object_object_get_ex(baseObj, "url", &urlObj))
        {
            success = false;
            LogError("unknown json format");
            goto cleanup;
        }

        if(!json_object_object_get_ex(baseObj, "login", &loginObj))
        {
            success = false;
            LogError("unknown json format");
            goto cleanup;
        }

        if(!json_object_object_get_ex(baseObj, "password", &passObj))
        {
            success = false;
            LogError("unknown json format");
            goto cleanup;
        }

        string_copy_into_cstr(project->basePath.ftp.path, json_object_get_string(pathObj));
        string_copy_into_cstr(project->basePath.ftp.url, json_object_get_string(urlObj));
        string_copy_into_cstr(project->basePath.ftp.login, json_object_get_string(loginObj));
        string_copy_into_cstr(project->basePath.ftp.password, json_object_get_string(passObj));
    }

cleanup:
    json_object_put(root);
    project->dirty = false;
    return success;
}

void SaveProject(struct Project *project)
{
    json_object *root = json_object_new_object();

    json_object_object_add(root, "version", json_object_new_int(PROJECT_VERSION));
    json_object_object_add(root, "type", json_object_new_int(project->basePath.type));
    json_object_object_add(root, "textures", json_object_new_string_len(project->texturesPath, string_length(project->texturesPath)));
    json_object_object_add(root, "things", json_object_new_string_len(project->thingsFile, string_length(project->thingsFile)));

    json_object *base = json_object_new_object();
    if(project->basePath.type == ASSPATH_FS)
    {
        json_object_object_add(base, "path", json_object_new_string_len(project->basePath.fs.path, string_length(project->basePath.fs.path)));
    }
    else
    {
        json_object_object_add(base, "url", json_object_new_string_len(project->basePath.ftp.url, string_length(project->basePath.ftp.url)));
        json_object_object_add(base, "login", json_object_new_string_len(project->basePath.ftp.login, string_length(project->basePath.ftp.login)));
        json_object_object_add(base, "password", json_object_new_string_len(project->basePath.ftp.password, string_length(project->basePath.ftp.password)));
        json_object_object_add(base, "path", json_object_new_string_len(project->basePath.ftp.path, string_length(project->basePath.ftp.path)));
    }

    json_object_object_add(root, "base", base);

    FILE *file = fopen(project->file, "w");
    fprintf(file, "%s", json_object_to_json_string_ext(root, JSON_C_TO_STRING_PRETTY));
    fclose(file);

    json_object_put(root);
    project->dirty = false;
}

void FreeProject(struct Project *project)
{
    string_free(project->file);
    string_free(project->thingsFile);
    string_free(project->texturesPath);
    string_free(project->basePath.ftp.path);
    string_free(project->basePath.ftp.url);
    string_free(project->basePath.ftp.login);
    string_free(project->basePath.ftp.password);
}
