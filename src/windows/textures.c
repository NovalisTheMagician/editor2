#include "../gwindows.h"

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

static void LoadFolder(struct TextureCollection *tc, pstring folder, pstring baseFolder)
{
    DIR *dp = opendir(pstr_tocstr(folder));
    if(!dp)
    {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while((entry = readdir(dp)))
    {
        pstring fileName = pstr_cstr(entry->d_name);
        if(pstr_cmp(fileName, ".") == 0 || pstr_cmp(fileName, "..") == 0) continue;

        pstring filePath = pstr_alloc(256);
        pstr_format(&filePath, "{s}/{s}", folder, fileName);

        struct stat buf;
        int r = stat(pstr_tocstr(filePath), &buf);
        if(r == -1)
        {
            perror("stat");
            return;
        }

        if(S_ISDIR(buf.st_mode))
        {
            LoadFolder(tc, filePath, baseFolder);
        }
        else if(S_ISREG(buf.st_mode))
        {
            ssize_t extIdx = pstr_last_index_of(filePath, ".");
            if(extIdx != -1)
            {
                pstring name = pstr_substring(filePath, baseFolder.size+1, extIdx);
                if(!tc_load(tc, name, filePath, buf.st_mtime))
                {
                    printf("failed to load texture: %s\n", pstr_tocstr(fileName));
                }
            }
        }

        pstr_free(fileName);
        pstr_free(filePath);
    }

    closedir(dp);
}

static void LoadTextures(struct TextureCollection *tc, struct Project *project, bool refresh)
{
    if(project->basePath.type != ASSPATH_FS) return;

    if(!refresh)
        tc_unload_all(tc);

    pstring textureFolder = pstr_alloc(256);

    if(project->basePath.fs.path.data[0] == '\0')
        pstr_format(&textureFolder, "{s}", project->texturesPath);
    else
        pstr_format(&textureFolder, "{s}/{s}", project->basePath.fs.path, project->texturesPath);

    LoadFolder(tc, textureFolder, textureFolder);
    pstr_free(textureFolder);
}

static void TextureIteration(struct Texture *texture, void *user)
{
    ImVec2 size = { .x = texture->width, .y = texture->height };
    igImageButton(pstr_tocstr(texture->name), (void*)(intptr_t)texture->texture1, size, (ImVec2){ 0, 0 }, (ImVec2){ 1, 1 }, (ImVec4){ 1, 1, 1, 1 }, (ImVec4){ 1, 1, 1, 1 });
    if (igIsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        igSetTooltip(pstr_tocstr(texture->name));
    }
}

struct Texture TexturesWindow(bool *p_open, struct EdState *state, bool popup)
{
    igSetNextWindowSize((ImVec2){ 800, 600 }, ImGuiCond_FirstUseEver);

    bool open = popup ? igBeginPopup("textures_popup", ImGuiWindowFlags_MenuBar) : igBegin("Texture Browser", p_open, ImGuiWindowFlags_MenuBar);
    if(open)
    {
        if(igBeginMenuBar())
        {
            if(igMenuItem_Bool("Refresh", "", false, true)) 
            {
                LoadTextures(&state->textures, &state->project, true);
            }
            if(igMenuItem_Bool("Invalidate", "", false, true))
            {
                LoadTextures(&state->textures, &state->project, false);
            }
            igEndMenuBar();
        }

        if(igInputText("Filter", state->data.textureFilter.data, state->data.textureFilter.capacity, 0, NULL, NULL))
        {
            state->data.textureFilter.size = strlen(pstr_tocstr(state->data.textureFilter));
        }

        if(igBeginChild_ID(2002, (ImVec2){ 0, 0 }, true, 0))
        {
            ImVec2 clientArea;
            igGetContentRegionAvail(&clientArea);

            if(state->data.textureFilter.data[0] == '\0')
                tc_iterate(&state->textures, TextureIteration, state);
            else
                tc_iterate_filter(&state->textures, TextureIteration, state->data.textureFilter, state);

            igEndChild();
        }
    }
    popup ? igEndPopup() : igEnd();

    return (struct Texture) { 0 };
}
