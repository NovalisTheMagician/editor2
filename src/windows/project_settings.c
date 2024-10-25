#include "../gwindows.h"
#include "../dialogs.h"
#include "cimgui.h"

#include <ftplib.h>
#include <string.h>

static void clearSourceFields(AssetPath *source)
{
    uint32_t type = source->type;
    memset(source, 0, sizeof *source);
    source->type = type;
}

static void BaseFsFields(EdState *state)
{
    if(igInputText("Path", state->project.basePath.fs.path, sizeof state->project.basePath.fs.path, 0, NULL, NULL))
    {
        state->project.dirty = true;
    }

    igSameLine(0, 8);
    if(igButton("Browse", (ImVec2){ 0, 0 }))
    {
        OpenFolderDialog(state->project.basePath.fs.path);
        state->project.dirty = true;
    }
}

static void BaseFtpFields(EdState *state, bool resetCheck)
{
    static const char *ftpStatusText = "";
    if(resetCheck) ftpStatusText = "";

    bool hasUrl = strlen(state->project.basePath.ftp.url);
    igSameLine(0, 8);
    igBeginDisabled(!hasUrl);
    if(igButton("Check Connection", (ImVec2){ 0, 0 }))
    {
        netbuf *handle;
        bool connected = FtpConnect(state->project.basePath.ftp.url, &handle);
        if(!connected)
        {
            ftpStatusText = "Failed to connect!";
            goto skip;
        }
        if(!FtpLogin(state->project.basePath.ftp.login, state->project.basePath.ftp.password, handle))
        {
            ftpStatusText = "Failed to login!";
            goto skip;
        }
        if(strlen(state->project.basePath.ftp.path) > 0 && !FtpChdir(state->project.basePath.ftp.path, handle))
        {
            ftpStatusText = "Couldn't find path!";
            goto skip;
        }
        ftpStatusText = "Success!";
skip:
        if(connected)
            FtpQuit(handle);
    }
    igEndDisabled();
    igSameLine(0, 8);
    igText(ftpStatusText);

    if(igInputText("URL", state->project.basePath.ftp.url, sizeof state->project.basePath.ftp.url, 0, NULL, NULL))
    {
        state->project.dirty = true;
        ftpStatusText = "";
    }
    if(igInputText("Login", state->project.basePath.ftp.login, sizeof state->project.basePath.ftp.login, 0, NULL, NULL))
    {
        state->project.dirty = true;
        ftpStatusText = "";
    }
    if(igInputText("Password", state->project.basePath.ftp.password, sizeof state->project.basePath.ftp.password, ImGuiInputTextFlags_Password, NULL, NULL))
    {
        state->project.dirty = true;
        ftpStatusText = "";
    }

    if(igInputText("Path", state->project.basePath.ftp.path, sizeof state->project.basePath.ftp.path, 0, NULL, NULL))
    {
        state->project.dirty = true;
        ftpStatusText = "";
    }
}

void ProjectSettingsWindow(bool *p_open, EdState *state)
{
    igSetNextWindowSize((ImVec2){ 400, 290 }, ImGuiCond_FirstUseEver);
    ImGuiWindowFlags flags = state->project.dirty ? ImGuiWindowFlags_UnsavedDocument : 0;
    if(igBegin("Project Settings", p_open, ImGuiWindowFlags_NoDocking | flags))
    {
        igSeparatorText("Base");
        bool isFtp = state->project.basePath.type == ASSPATH_FTP;
        bool resetCheck = false;
        if(igCheckbox("FTP", &isFtp))
        {
            //clearSourceFields(&state->project.basePath);
            resetCheck = true;
        }
        state->project.basePath.type = isFtp ? ASSPATH_FTP : ASSPATH_FS;

        if(isFtp)
            BaseFtpFields(state, resetCheck);
        else
            BaseFsFields(state);

        igSeparatorText("Textures");
        igPushID_Str("Textures");
        if(igInputText("Subpath", state->project.texturesPath, sizeof state->project.texturesPath, 0, NULL, NULL))
        {
            state->project.dirty = true;
        }
        igPopID();

        igSeparatorText("Things");
        igPushID_Str("Things");
        if(igInputText("File", state->project.thingsFile, sizeof state->project.thingsFile, 0, NULL, NULL))
        {
            state->project.dirty = true;
        }
        igPopID();
    }
    igEnd();
}
