#include "editor.h"

#include <string.h> // IWYU pragma: keep

const char* ColorIndexToString(enum Colors color)
{
    switch(color)
    {
    case COL_LOG_INFO: return "Log Info Text";
    case COL_LOG_WARN: return "Log Warning Text";
    case COL_LOG_ERRO: return "Log Error Text";
    case COL_LOG_DEBU: return "Log Debug Text";
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
    default: return "Unknown";
    }
}

void ResetSettings(EdSettings *settings)
{
    settings->colors[COL_LOG_INFO] = (Color){ .r = 0.95f, .g = 0.95f, .b = 0.95f, .a = 1.00f };
    settings->colors[COL_LOG_WARN] = (Color){ .r = 1.00f, .g = 1.00f, .b = 0.05f, .a = 1.00f };
    settings->colors[COL_LOG_ERRO] = (Color){ .r = 1.00f, .g = 0.00f, .b = 0.00f, .a = 1.00f };
    settings->colors[COL_LOG_DEBU] = (Color){ .r = 0.00f, .g = 0.75f, .b = 1.00f, .a = 1.00f };

    settings->colors[COL_WORKSPACE_BACK] = (Color){ .r = 0.45f, .g = 0.55f, .b = 0.60f, .a = 1.00f };

    settings->colors[COL_BACKGROUND] = (Color){ .r = 0.20f, .g = 0.20f, .b = 0.20f, .a = 1.00f };
    settings->colors[COL_BACK_LINES] = (Color){ .r = 0.25f,.g =  0.25f, .b = 0.25f, .a = 1.00f };
    settings->colors[COL_BACK_MAJOR_LINES] = (Color){ .r = 0.40f, .g = 0.40f, .b = 0.40f, .a = 1.00f };

    settings->colors[COL_RTBACKGROUND] = (Color){ .r = 0.00f, .g = 0.00f, .b = 0.00f, .a = 1.00f };

    settings->colors[COL_VERTEX] = (Color){ .r = 0.7f, .g = 0.7f, .b = 0.7f, .a = 1.00f };
    settings->colors[COL_LINE] = (Color){ .r = 0.8f, .g = 0.8f, .b = 0.8f, .a = 1.00f };
    settings->colors[COL_SECTOR] = (Color){ .r = 1.0f, .g = 1.0f, .b = 1.0f, .a = 1.0f };

    settings->colors[COL_ACTIVE_EDIT] = (Color){ .r = 0.8f, .g = 0.8f, .b = 0.3f, .a = 1.0f };

    settings->colors[COL_VERTEX_HOVER] = (Color){ .r = 1.0f, .g = 1.0f, .b = 1.0f, .a = 1.00f };
    settings->colors[COL_VERTEX_SELECT] = (Color){ .r = 1.0f, .g = 1.0f, .b = 0.0f, .a = 1.00f };

    settings->colors[COL_LINE_HOVER] = (Color){ .r = 1.0f, .g = 1.0f, .b = 1.0f, .a = 1.00f };
    settings->colors[COL_LINE_SELECT] = (Color){ .r = 1.0f, .g = 1.0f, .b = 0.0f, .a = 1.00f };

    settings->colors[COL_SECTOR_HOVER] = (Color){ .r = 0.8f, .g = 0.8f, .b = 0.8f, .a = 1.00f };
    settings->colors[COL_SECTOR_SELECT] = (Color){ .r = 0.8f, .g = 0.3f, .b = 0.3f, .a = 1.00f };

    settings->colors[COL_LINE_INNER] = (Color){ .r = 0.6f, .g = 0.6f, .b = 0.6f, .a = 1.00f };

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

bool LoadSettings(const char *settingsPath, EdSettings *settings)
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

        stringtok *tokenizer = stringtok_start(buffer);
        while(!stringtok_done(tokenizer))
        {
            size_t size;
            char *strline = stringtok_next(tokenizer, "\n", &size);
            pstring line = string_cstr_size(size+1, strline);

            ssize_t idx = string_first_index_of(line, 0, "=");
            if(idx == -1 || idx == 0)
            {
                string_free(line);
                continue;
            }

            pstring key = string_substring(line, 0, idx);
            pstring value = string_substring(line, idx + 1, -1);

            if(strcasecmp(key, "theme") == 0) settings->theme = atoi(value);
            else if(strcasecmp(key, "vertex_point_size") == 0) settings->vertexPointSize = (float)atof(value);
            else if(strcasecmp(key, "show_grid_lines") == 0) settings->showGridLines = atoi(value);
            else if(strcasecmp(key, "show_major_axis") == 0) settings->showMajorAxis = atoi(value);
            else if(strcasecmp(key, "preview_fov") == 0) settings->realtimeFov = atoi(value);
            else if(strcasecmp(key, "game_path") == 0) string_copy_into(settings->gamePath, value);
            else if(strcasecmp(key, "launch_arguments") == 0) string_copy_into(settings->launchArguments, value);

            string_free(key);
            string_free(value);
            string_free(line);
        }

        stringtok_end(tokenizer);
        string_free(buffer);

        fclose(settingsFile);
        return true;
    }
    return false;
}

void SaveSettings(const char *settingsPath, const EdSettings *settings)
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

void FreeSettings(EdSettings *settings)
{
    string_free(settings->gamePath);
    string_free(settings->launchArguments);
}
