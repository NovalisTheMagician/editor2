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

    SetColor(&settings->colors[COL_VERTEX], (Color){ 0.7f, 0.7f, 0.7f, 1.00f });
    SetColor(&settings->colors[COL_LINE], (Color){ 0.8f, 0.8f, 0.8f, 1.00f });
    SetColor(&settings->colors[COL_SECTOR], (Color){ 0.8f, 0.8f, 0.3f, 0.7f });

    pstr_free(settings->gamePath);
    pstr_free(settings->launchArguments);
    settings->gamePath = pstr_alloc(MAX_GAMEPATH_LEN);
    settings->launchArguments = pstr_cstr_size("-debug -map %1", MAX_GAMEPATH_LEN);

    settings->theme = 0;

    settings->showGridLines = true;
    settings->showMajorAxis = true;
    settings->vertexPointSize = 7.0f;

    settings->realtimeFov = 90;
}

bool LoadSettings(const char *settingsPath, struct EdSettings *settings)
{
    FILE *settingsFile = fopen(settingsPath, "r");
    if(settingsFile)
    {
        fseek(settingsFile, 0, SEEK_END);
        long size = ftell(settingsFile);
        rewind(settingsFile);

        pstring buffer = pstr_alloc(size);
        fread(buffer.data, 1, size, settingsFile);
        buffer.size = size;

        pstring lineBuf = buffer;
        do
        {
            pstring line = pstr_tok(&lineBuf, "\n");

            pstring key = pstr_tok(&line, "=");
            pstring value = line;

                 if(pstr_cmp(key, "theme") == 0) settings->theme = atoi(pstr_tocstr(value));
            else if(pstr_cmp(key, "vertex_point_size") == 0) settings->vertexPointSize = (float)atof(pstr_tocstr(value));
            else if(pstr_cmp(key, "show_grid_lines") == 0) settings->showGridLines = atoi(pstr_tocstr(value));
            else if(pstr_cmp(key, "show_major_axis") == 0) settings->showMajorAxis = atoi(pstr_tocstr(value));
            else if(pstr_cmp(key, "preview_fov") == 0) settings->realtimeFov = atoi(pstr_tocstr(value));
            else if(pstr_cmp(key, "game_path") == 0) pstr_copy_into(&settings->gamePath, value);
            else if(pstr_cmp(key, "launch_arguments") == 0) pstr_copy_into(&settings->launchArguments, value);
        }
        while(lineBuf.size > 0);

        pstr_free(buffer);

        fclose(settingsFile);
        return true;
    }
    return false;
}

void SaveSettings(const char *settingsPath, const struct EdSettings *settings)
{
    FILE *settingsFile = fopen(settingsPath, "w+");
    if(settingsFile)
    {
        fprintf(settingsFile, "theme=%d\n", settings->theme);
        fprintf(settingsFile, "vertex_point_size=%.2f\n", settings->vertexPointSize);
        fprintf(settingsFile, "show_grid_lines=%d\n", settings->showGridLines);
        fprintf(settingsFile, "show_major_axis=%d\n", settings->showMajorAxis);
        fprintf(settingsFile, "preview_fov=%d\n", settings->realtimeFov);
        fprintf(settingsFile, "game_path=%s\n", pstr_tocstr(settings->gamePath));
        fprintf(settingsFile, "launch_arguments=%s\n", pstr_tocstr(settings->launchArguments));
        fclose(settingsFile);
    }
}

void FreeSettings(struct EdSettings *settings)
{
    pstr_free(settings->gamePath);
    pstr_free(settings->launchArguments);
}
