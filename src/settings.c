#include "editor.h"
#include "logging.h"

#include <string.h>
#include "serialization.h"
#include "utils/string.h"

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
    settings->colors[COL_LINE] = (Color){ .r = 1.0f, .g = 1.0f, .b = 1.0f, .a = 1.00f };
    settings->colors[COL_SECTOR] = (Color){ .r = 1.0f, .g = 1.0f, .b = 1.0f, .a = 1.0f };

    settings->colors[COL_ACTIVE_EDIT] = (Color){ .r = 0.8f, .g = 0.8f, .b = 0.3f, .a = 1.0f };

    settings->colors[COL_VERTEX_HOVER] = (Color){ .r = 1.0f, .g = 1.0f, .b = 1.0f, .a = 1.00f };
    settings->colors[COL_VERTEX_SELECT] = (Color){ .r = 1.0f, .g = 1.0f, .b = 0.0f, .a = 1.00f };

    settings->colors[COL_LINE_HOVER] = (Color){ .r = 1.0f, .g = 0.5f, .b = 0.0f, .a = 1.00f };
    settings->colors[COL_LINE_SELECT] = (Color){ .r = 0.0f, .g = 1.0f, .b = 0.0f, .a = 1.00f };

    settings->colors[COL_SECTOR_HOVER] = (Color){ .r = 0.8f, .g = 0.8f, .b = 0.8f, .a = 1.00f };
    settings->colors[COL_SECTOR_SELECT] = (Color){ .r = 0.8f, .g = 0.3f, .b = 0.3f, .a = 1.00f };

    settings->colors[COL_LINE_INNER] = (Color){ .r = 0.6f, .g = 0.6f, .b = 0.6f, .a = 1.00f };

    memset(settings->gamePath, 0, sizeof settings->gamePath);
    strncpy(settings->launchArguments, "-debug -map %1", MAX_GAMEPATH_LEN);

    settings->theme = 0;

    settings->showGridLines = true;
    settings->showMajorAxis = true;
    settings->vertexPointSize = 7.0f;

    settings->realtimeFov = 90;
}

bool LoadSettings(const char *settingsPath, EdSettings *settings)
{
    FILE *file = fopen(settingsPath, "r");
    if(file)
    {

        int lineNr = 0;
        char lineRaw[1024] = { 0 };
        while(fgets(lineRaw, sizeof lineRaw, file))
        {
            lineNr++;
            char *line = Trim(lineRaw);
            if(line[0] == '/' && line[1] == '/') continue; // comment

            char *delim = strchr(line, '=');
            if(!delim || delim == line)
                goto parseError;

            *delim = '\0';
            char *key = Trim(line);
            char *value = Trim(delim+1);

            if(strcasecmp(key, "theme") == 0) { if(ParseInt(value, &settings->theme)) continue; }
            else if(strcasecmp(key, "vertex_point_size") == 0) { if(ParseFloat(value, &settings->vertexPointSize)) continue; }
            else if(strcasecmp(key, "show_grid_lines") == 0) { if(ParseBool(value, &settings->showGridLines)) continue; }
            else if(strcasecmp(key, "show_major_axis") == 0) { if(ParseBool(value, &settings->showMajorAxis)) continue; }
            else if(strcasecmp(key, "show_line_dir") == 0) { if(ParseBool(value, &settings->showLineDir)) continue; }
            else if(strcasecmp(key, "preview_fov") == 0) { if(ParseInt(value, &settings->realtimeFov)) continue; }
            else if(strcasecmp(key, "game_path") == 0) { strncpy(settings->gamePath, value, sizeof settings->gamePath); continue; }
            else if(strcasecmp(key, "launch_arguments") == 0) { strncpy(settings->launchArguments, value, sizeof settings->launchArguments); continue; }
parseError:
            LogWarning("Failed to parse `%s` on line: %d", settingsPath, lineNr);
        }

        fclose(file);
        return true;
    }
    return false;
}

void SaveSettings(const char *settingsPath, const EdSettings *settings)
{
    FILE *file = fopen(settingsPath, "w+");
    if(file)
    {
        fprintf(file, "// Themes\n");
        fprintf(file, "theme=%d\n", settings->theme);

        fprintf(file, "// Editor\n");
        fprintf(file, "vertex_point_size=%.2f\n", settings->vertexPointSize);
        fprintf(file, "show_grid_lines=%d\n", settings->showGridLines);
        fprintf(file, "show_major_axis=%d\n", settings->showMajorAxis);
        fprintf(file, "show_line_dir=%d\n", settings->showLineDir);

        fprintf(file, "// 3D View\n");
        fprintf(file, "preview_fov=%d\n", settings->realtimeFov);

        fprintf(file, "// Base\n");
        fprintf(file, "game_path=%s\n", settings->gamePath);
        fprintf(file, "launch_arguments=%s\n", settings->launchArguments);
        fclose(file);
    }
}

void FreeSettings(EdSettings *)
{
}
