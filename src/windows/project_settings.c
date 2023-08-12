#include "../windows.h"
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
        if(igInputText("Path", state->project.basePath.fs.path.data, state->project.basePath.fs.path.size, 0, NULL, NULL)) { state->project.dirty = true; }
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
            if(igInputText("URL", state->project.basePath.ftp.url.data, state->project.basePath.ftp.url.size, 0, NULL, NULL)) { state->project.dirty = true; }
            if(igInputText("Login", state->project.basePath.ftp.login.data, state->project.basePath.ftp.login.size, 0, NULL, NULL)) { state->project.dirty = true; }
            if(igInputText("Password", state->project.basePath.ftp.password.data, state->project.basePath.ftp.password.size, 0, NULL, NULL)) { state->project.dirty = true; }
        }

        igSeparatorText("Textures");
        igPushID_Str("Textures");
        if(igInputText("Subpath", state->project.texturesPath.data, state->project.texturesPath.size, 0, NULL, NULL)) { state->project.dirty = true; }
        igPopID();

        igSeparatorText("Things");
        igPushID_Str("Things");
        if(igInputText("Subpath", state->project.thingsFile.data, state->project.thingsFile.size, 0, NULL, NULL)) { state->project.dirty = true; }
        igPopID();
    }
    igEnd();
}
