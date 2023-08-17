#include "project.h"

#include <json-c/json.h>

#define PROJECT_VERSION 1

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
    project->texturesPath = pstr_cstr_size("textures", MAX_ASSETPATH_LEN);
    project->thingsFile = pstr_cstr_size("things.txt", MAX_ASSETPATH_LEN);

    project->dirty = false;
}

bool LoadProject(struct Project *project)
{
    json_object *root = json_object_from_file(pstr_tocstr(project->file));
    if(!root)
    {
        return false;
    }

    bool success = true;

    json_object *versionObj, *typeObj, *texturesPathObj, *thingsObj, *baseObj;
    if(!json_object_object_get_ex(root, "version", &versionObj))
    {
        success = false;
        printf("unknown json format\n");
        goto cleanup;
    }

    if(!json_object_object_get_ex(root, "type", &typeObj))
    {
        success = false;
        printf("unknown json format\n");
        goto cleanup;
    }

    if(!json_object_object_get_ex(root, "textures", &texturesPathObj))
    {
        success = false;
        printf("unknown json format\n");
        goto cleanup;
    }

    if(!json_object_object_get_ex(root, "things", &thingsObj))
    {
        success = false;
        printf("unknown json format\n");
        goto cleanup;
    }

    if(!json_object_object_get_ex(root, "base", &baseObj))
    {
        success = false;
        printf("unknown json format\n");
        goto cleanup;
    }

    int version = json_object_get_int(versionObj);
    if(version != PROJECT_VERSION)
    {
        success = false;
        printf("project version mismatch\n");
        goto cleanup;
    }

    project->basePath.type = json_object_get_int(typeObj);
    if(project->basePath.type != ASSPATH_FS && project->basePath.type != ASSPATH_FTP)
    {
        success = false;
        printf("invalid base type\n");
        goto cleanup;
    }

    pstr_copy_into(&project->texturesPath, json_object_get_string(texturesPathObj));
    pstr_copy_into(&project->thingsFile, json_object_get_string(thingsObj));

    if(project->basePath.type == ASSPATH_FS)
    {
        json_object *pathObj;
        if(!json_object_object_get_ex(baseObj, "path", &pathObj))
        {
            success = false;
            printf("unknown json format\n");
            goto cleanup;
        }

        pstr_copy_into(&project->basePath.fs.path, json_object_get_string(pathObj));
    }
    else
    {
        json_object *pathObj, *urlObj, *loginObj, *passObj;
        if(!json_object_object_get_ex(baseObj, "path", &pathObj))
        {
            success = false;
            printf("unknown json format\n");
            goto cleanup;
        }

        if(!json_object_object_get_ex(baseObj, "url", &urlObj))
        {
            success = false;
            printf("unknown json format\n");
            goto cleanup;
        }

        if(!json_object_object_get_ex(baseObj, "login", &loginObj))
        {
            success = false;
            printf("unknown json format\n");
            goto cleanup;
        }

        if(!json_object_object_get_ex(baseObj, "password", &passObj))
        {
            success = false;
            printf("unknown json format\n");
            goto cleanup;
        }

        pstr_copy_into(&project->basePath.ftp.path, json_object_get_string(pathObj));
        pstr_copy_into(&project->basePath.ftp.url, json_object_get_string(urlObj));
        pstr_copy_into(&project->basePath.ftp.login, json_object_get_string(loginObj));
        pstr_copy_into(&project->basePath.ftp.password, json_object_get_string(passObj));
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
    json_object_object_add(root, "textures", json_object_new_string_len(project->texturesPath.data, project->texturesPath.size));
    json_object_object_add(root, "things", json_object_new_string_len(project->thingsFile.data, project->thingsFile.size));

    json_object *base = json_object_new_object();
    if(project->basePath.type == ASSPATH_FS)
    {
        json_object_object_add(base, "path", json_object_new_string_len(project->basePath.fs.path.data, project->basePath.fs.path.size));
    }
    else
    {
        json_object_object_add(base, "url", json_object_new_string_len(project->basePath.ftp.url.data, project->basePath.ftp.url.size));
        json_object_object_add(base, "login", json_object_new_string_len(project->basePath.ftp.login.data, project->basePath.ftp.login.size));
        json_object_object_add(base, "password", json_object_new_string_len(project->basePath.ftp.password.data, project->basePath.ftp.password.size));
        json_object_object_add(base, "path", json_object_new_string_len(project->basePath.ftp.path.data, project->basePath.ftp.path.size));
    }

    json_object_object_add(root, "base", base);

    FILE *file = fopen(pstr_tocstr(project->file), "w");
    fprintf(file, "%s", json_object_to_json_string_ext(root, JSON_C_TO_STRING_PRETTY));
    fclose(file);

    json_object_put(root);
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
