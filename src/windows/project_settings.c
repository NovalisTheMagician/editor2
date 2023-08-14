#include "../gwindows.h"
#include "../dialogs.h"

void ProjectSettingsWindow(bool *p_open, struct EdState *state)
{
    igSetNextWindowSize((ImVec2){ 400, 290 }, ImGuiCond_FirstUseEver);
    if(igBegin("Project Settings", p_open, 0))
    {

        igSeparatorText("Base");
        bool isFtp = state->project.basePath.type == ASSPATH_FTP;
        igCheckbox("FTP", &isFtp);
        state->project.basePath.type = isFtp ? ASSPATH_FTP : ASSPATH_FS;
        if(isFtp)
        {
            igSameLine(0, 8);
            if(igButton("Check Connection", (ImVec2){ 0, 0 }))
            {

            }
        }
        if(igInputText_pstr("Path", &state->project.basePath.fs.path, 0, NULL, NULL)) 
        {
            state->project.dirty = true;
        }
        igSameLine(0, 8);
        if(igButton("Browse", (ImVec2){ 0, 0 }))
        {
            if(!isFtp)
            {
                OpenFolderDialog(&state->project.basePath.fs.path);
                state->project.dirty = true;
            }
        }
        if(isFtp)
        {
            if(igInputText_pstr("URL", &state->project.basePath.ftp.url, 0, NULL, NULL)) 
            {
                state->project.dirty = true;
            }
            if(igInputText_pstr("Login", &state->project.basePath.ftp.login, 0, NULL, NULL)) 
            {
                state->project.dirty = true;
            }
            if(igInputText_pstr("Password", &state->project.basePath.ftp.password, 0, NULL, NULL)) 
            {
                state->project.dirty = true;
            }
        }

        igSeparatorText("Textures");
        igPushID_Str("Textures");
        if(igInputText_pstr("Subpath", &state->project.texturesPath, 0, NULL, NULL)) 
        {
            state->project.dirty = true;
        }
        igPopID();

        igSeparatorText("Things");
        igPushID_Str("Things");
        if(igInputText_pstr("File", &state->project.thingsFile, 0, NULL, NULL)) 
        {
            state->project.dirty = true;
        }
        igPopID();
    }
    igEnd();
}
