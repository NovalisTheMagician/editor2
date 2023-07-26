#include "editor.h"

const char* ColorIndexToString(enum Colors color)
{
    switch(color)
    {
    case COL_WORKSPACE_BACK: return "Workspace Background";
    case COL_BACKGROUND: return "Editor Background";
    case COL_RTBACKGROUND: return "3D View Background";
    case COL_BACK_LINES: return "Editor Background Lines";
    case COL_VERTEX: return "Vertices";
    case COL_LINE: return "Lines";
    case COL_SECTOR: return "Sectors";
    case COL_BACK_MAJOR_LINES: return "Editor Background Major Axis";
    default: return "Unkown";
    }
}

void ResetSettings(struct EdSettings *settings)
{
    SetColor(&settings->colors[COL_WORKSPACE_BACK], (Color){ 0.45f, 0.55f, 0.60f, 1.00f });

    SetColor(&settings->colors[COL_BACKGROUND], (Color){ 0.2f, 0.2f, 0.2f, 1.00f });
    SetColor(&settings->colors[COL_BACK_LINES], (Color){ 0.25f, 0.25f, 0.25f, 1.00f });
    SetColor(&settings->colors[COL_BACK_MAJOR_LINES], (Color){ 0.4f, 0.4f, 0.4f, 1.00f });

    SetColor(&settings->colors[COL_RTBACKGROUND], (Color){ 0.00f, 0.00f, 0.00f, 1.00f });

    SetColor(&settings->colors[COL_VERTEX], (Color){ 0.8f, 0.8f, 0.8f, 1.00f });
    SetColor(&settings->colors[COL_LINE], (Color){ 0.8f, 0.8f, 0.8f, 1.00f });
    SetColor(&settings->colors[COL_SECTOR], (Color){ 0.8f, 0.8f, 0.8f, 1.00f });

    pstr_free(settings->gamePath);
    pstr_free(settings->launchArguments);
    settings->gamePath = pstr_alloc(MAX_GAMEPATH_LEN);
    settings->launchArguments = pstr_cstr_size("-debug -map %1", MAX_GAMEPATH_LEN);

    settings->theme = 0;
}

bool LoadSettings(const char *settingsPath, struct EdSettings *settings)
{
    return false;
}

void SaveSettings(const char *settingsPath, const struct EdSettings *settings)
{

}

void FreeSettings(struct EdSettings *settings)
{
    pstr_free(settings->gamePath);
    pstr_free(settings->launchArguments);
}
