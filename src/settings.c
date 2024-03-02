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
    case COL_ACTIVE_EDIT: return "Active Edit Data";
    case COL_VERTEX_HOVER: return "Hovered Vertex";
    case COL_VERTEX_SELECT: return "Selected Vertex";
    case COL_LINE_HOVER: return "Hovered Line";
    case COL_LINE_SELECT: return "Selected Line";
    case COL_SECTOR_HOVER: return "Hovered Sector";
    case COL_SECTOR_SELECT: return "Selected Sector";
    case COL_LINE_INNER: return "Inner Line";
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
    SetColor(&settings->colors[COL_SECTOR], (Color){ 0.3f, 0.8f, 0.3f, 0.7f });

    SetColor(&settings->colors[COL_ACTIVE_EDIT], (Color){ 0.8f, 0.8f, 0.3f, 1.0f });

    SetColor(&settings->colors[COL_VERTEX_HOVER], (Color){ 1.0f, 1.0f, 1.0f, 1.00f });
    SetColor(&settings->colors[COL_VERTEX_SELECT], (Color){ 1.0f, 1.0f, 0.0f, 1.00f });

    SetColor(&settings->colors[COL_LINE_HOVER], (Color){ 1.0f, 1.0f, 1.0f, 1.00f });
    SetColor(&settings->colors[COL_LINE_SELECT], (Color){ 1.0f, 1.0f, 0.0f, 1.00f });

    SetColor(&settings->colors[COL_SECTOR_HOVER], (Color){ 0.4f, 0.9f, 0.4f, 1.00f });
    SetColor(&settings->colors[COL_SECTOR_SELECT], (Color){ 1.0f, 1.0f, 0.0f, 1.00f });

    SetColor(&settings->colors[COL_LINE_INNER], (Color){ 0.6f, 0.6f, 0.6f, 1.00f });

    string_free(settings->gamePath);
    string_free(settings->launchArguments);
    settings->gamePath = string_alloc(MAX_GAMEPATH_LEN);
    settings->launchArguments = string_cstr_alloc("-debug -map %1", MAX_GAMEPATH_LEN);

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

        pstring buffer = string_alloc(size);
        fread(buffer, 1, size, settingsFile);
        string_recalc(buffer);

        struct stringtok *tokenizer = stringtok_start(buffer);
        while(!stringtok_done(tokenizer))
        {
            size_t size;
            char *strline = stringtok_next(tokenizer, "\n", &size);
            pstring line = string_cstr_size(size, strline);

            ssize_t idx = string_first_index_of(line, 0, "=");
            if(idx == -1 || idx == 0)
            {
                string_free(line);
                continue;
            }

            char *key = pstr_tok(&line, "=");
            char *value = line;

                 if(strcasecmp(key, "theme") == 0) settings->theme = atoi(pstr_tocstr(value));
            else if(strcasecmp(key, "vertex_point_size") == 0) settings->vertexPointSize = (float)atof(pstr_tocstr(value));
            else if(strcasecmp(key, "show_grid_lines") == 0) settings->showGridLines = atoi(pstr_tocstr(value));
            else if(strcasecmp(key, "show_major_axis") == 0) settings->showMajorAxis = atoi(pstr_tocstr(value));
            else if(strcasecmp(key, "preview_fov") == 0) settings->realtimeFov = atoi(pstr_tocstr(value));
            else if(strcasecmp(key, "game_path") == 0) pstr_copy_into(&settings->gamePath, value);
            else if(strcasecmp(key, "launch_arguments") == 0) pstr_copy_into(&settings->launchArguments, value);
        }

        stringtok_end(tokenizer);
        string_free(buffer);

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
        fprintf(settingsFile, "game_path=%s\n", settings->gamePath);
        fprintf(settingsFile, "launch_arguments=%s\n", settings->launchArguments);
        fclose(settingsFile);
    }
}

void FreeSettings(struct EdSettings *settings)
{
    string_free(settings->gamePath);
    string_free(settings->launchArguments);
}
