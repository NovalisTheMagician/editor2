#pragma once

#include "common.h"
#include "map.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>

#define MAX_GAMEPATH_LEN 256
#define MAX_GAMEARGUMENTS_LEN 256

enum Theme
{
    THEME_IMGUI_LIGHT,
    THEME_IMGUI_DARK,
    THEME_IMGUI_CLASSIC,
    THEME_VALVE,
    THEME_DEUSEX,

    NUM_THEMES
};

enum Colors
{
    COL_WORKSPACE_BACK,

    COL_BACKGROUND,
    COL_BACK_LINES,

    COL_RTBACKGROUND,

    COL_VERTEX,
    COL_LINE,
    COL_SECTOR,

    NUM_COLORS
};

enum SelectionMode
{
    MODE_VERTEX,
    MODE_LINE,
    MODE_SECTOR
};

typedef float Color[4];
static inline void SetColor(Color *to, Color from)
{
    (*to)[0] = from[0];
    (*to)[1] = from[1];
    (*to)[2] = from[2];
    (*to)[3] = from[3];
}

struct EdSettings
{
    Color colors[NUM_COLORS];
    int theme;

    char gamePath[MAX_GAMEPATH_LEN];
    char launchArguments[MAX_GAMEARGUMENTS_LEN];
};

struct EdState
{
    struct EdSettings settings;

    struct
    {
        bool showMetrics;
        bool showAbout;

        bool showToolbar;
        bool showTextures;
        bool showEntities;
        bool show3dView;
        bool showLogs;

        bool showBuildLog;

        bool showSettings;
        bool showMapSettings;
        bool showProjectSettings;

        int gridSize;
        float zoomLevel;
        ImVec2 viewPosition;
        int selectionMode;
    } ui;

    struct
    {
        int editorFramebufferWidth;
        int editorFramebufferHeight;
        GLuint editorFramebuffer;
        GLuint editorColorTexture;

        int realtimeFramebufferWidth;
        int realtimeFramebufferHeight;
        GLuint realtimeFramebuffer;
        GLuint realtimeColorTexture;
        GLuint realtimeDepthTexture;

        GLuint backgroundLinesBuffer;

        struct 
        {
            GLuint hProgram, vProgram;
            GLuint hOffsetUniform, vOffsetUniform, hPeriodUniform, vPeriodUniform, hTintUniform, vTintUniform;
            GLuint hVPUniform, vVPUniform;
            GLuint backVertexFormat;
        } editorBackProg;
    } gl;

    float realtimeFov;
    vec2 backOffset;

    mat4 editorView, editorProjection;
    mat4 realtimeView, realtimeProjection;
};

const char* ColorIndexToString(enum Colors color);

void ResetSettings(struct EdSettings *settings);
bool LoadSettings(const char *settingsPath, struct EdSettings *settings);
void SaveSettings(const char *settingsPath, const struct EdSettings *settings);

bool InitEditor(struct EdState *state);
void DestroyEditor(struct EdState *state);

void ResizeEditorView(struct EdState *state, int width, int height);
void ResizeRealtimeView(struct EdState *state, int width, int height);

void RenderEditorView(const struct EdState *state, const struct Map *map);
void RenderRealtimeView(const struct EdState *state, const struct Map *map);
