#include "../gwindows.h"
#include "../dialogs.h"

#include <ftplib.h>

static void BaseFsFields(struct EdState *state)
{
    if(igInputText_pstr("Path", &state->project.basePath.fs.path, 0, NULL, NULL))
    {
        state->project.dirty = true;
    }

    igSameLine(0, 8);
    if(igButton("Browse", (ImVec2){ 0, 0 }))
    {
        OpenFolderDialog(&state->project.basePath.fs.path);
        state->project.dirty = true;
    }
}

static void BaseFtpFields(struct EdState *state, bool resetCheck)
{
    static const char *ftpStatusText = "";
    if(resetCheck) ftpStatusText = "";

    igSameLine(0, 8);
    if(igButton("Check Connection", (ImVec2){ 0, 0 }))
    {
        netbuf *handle;
        if(!FtpConnect(pstr_tocstr(state->project.basePath.ftp.url), &handle))
        {
            ftpStatusText = "Failed to connect!";
            goto skip;
        }
        if(!FtpLogin(pstr_tocstr(state->project.basePath.ftp.login), pstr_tocstr(state->project.basePath.ftp.password), handle))
        {
            ftpStatusText = "Failed to login!";
            goto skip;
        }
        if(state->project.basePath.ftp.path.size > 0)
        if(!FtpChdir(pstr_tocstr(state->project.basePath.ftp.path), handle))
        {
            ftpStatusText = "Couldn't find path!";
            goto skip;
        }
        ftpStatusText = "Success!";
skip:
        FtpQuit(handle);
    }
    igSameLine(0, 8);
    igText(ftpStatusText);

    if(igInputText_pstr("URL", &state->project.basePath.ftp.url, 0, NULL, NULL))
    {
        state->project.dirty = true;
        ftpStatusText = "";
    }
    if(igInputText_pstr("Login", &state->project.basePath.ftp.login, 0, NULL, NULL))
    {
        state->project.dirty = true;
        ftpStatusText = "";
    }
    if(igInputText_pstr("Password", &state->project.basePath.ftp.password, ImGuiInputTextFlags_Password, NULL, NULL))
    {
        state->project.dirty = true;
        ftpStatusText = "";
    }

    if(igInputText_pstr("Path", &state->project.basePath.ftp.path, 0, NULL, NULL))
    {
        state->project.dirty = true;
        ftpStatusText = "";
    }
}

void ProjectSettingsWindow(bool *p_open, struct EdState *state)
{
    igSetNextWindowSize((ImVec2){ 400, 290 }, ImGuiCond_FirstUseEver);
    if(igBegin("Project Settings", p_open, 0))
    {
        igSeparatorText("Base");
        bool isFtp = state->project.basePath.type == ASSPATH_FTP;
        bool resetCheck = false;
        if(igCheckbox("FTP", &isFtp)) resetCheck = true;
        state->project.basePath.type = isFtp ? ASSPATH_FTP : ASSPATH_FS;
        
        if(isFtp)
            BaseFtpFields(state, resetCheck);
        else
            BaseFsFields(state);

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
