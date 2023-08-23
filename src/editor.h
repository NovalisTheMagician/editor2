#pragma once

#include "common.h"
#include "map.h"
#include "project.h"
#include "network.h"
#include "texture_collection.h"
#include "async_load.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>

#define MAX_GAMEPATH_LEN 256
#define MAX_GAMEARGUMENTS_LEN 256

#define MAX_ZOOM 10.0f
#define MIN_ZOOM 0.05f

#define MAX_FOV 120
#define MIN_FOV 45

#define MAX_VERTEXPOINTSIZE 20
#define MIN_VERTEXPOINTSIZE 0.5f

#define TEXTURE_FILTER_LEN 256

#define LINE_BUFFER_CAP 256

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
    COL_BACK_MAJOR_LINES,

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

    float vertexPointSize;
    bool showGridLines, showMajorAxis;
    int realtimeFov;

    pstring gamePath;
    pstring launchArguments;
};

struct VertexType
{
    vec2 position;
    Color color;
};

struct SectorVertexType
{
    vec2 position;
    Color color;
    vec2 texCoord;
};

typedef uint32_t Index_t;

struct EdState
{
    struct EdSettings settings;
    struct Map map;
    struct Project project;
    struct Network network;
    struct TextureCollection textures;
    struct AsyncJob async;

    struct
    {
        bool showMetrics;
        bool showAbout;

        bool showToolbar;
        bool showTextures;
        bool showEntities;
        bool show3dView;
        bool showLogs;
        bool showStats;

        bool showBuildLog;

        bool showSettings;
        bool showMapSettings;
        bool showProjectSettings;
    } ui;

    struct
    {
        int editorFramebufferWidth;
        int editorFramebufferHeight;
        GLuint editorFramebuffer, editorFramebufferMS;
        GLuint editorColorTexture, editorColorTextureMS;

        int realtimeFramebufferWidth;
        int realtimeFramebufferHeight;
        GLuint realtimeFramebuffer;
        GLuint realtimeColorTexture;
        GLuint realtimeDepthTexture;

        GLuint backgroundLinesBuffer;

        GLuint whiteTexture;

        struct 
        {
            GLuint hProgram, vProgram;
            GLuint hOffsetUniform, vOffsetUniform, hPeriodUniform, vPeriodUniform, hTintUniform, vTintUniform;
            GLuint hMajorTintUniform, hMajorIdxUniform, vMajorTintUniform, vMajorIdxUniform;
            GLuint hVPUniform, vVPUniform;
            GLuint backVertexFormat;
        } editorBackProg;

        struct
        {
            GLuint program;
            GLuint viewProjUniform, tintUniform;
            GLuint vertBuffer;
            GLuint vertFormat;
            struct VertexType *bufferMap;
        } editorVertex;

        struct
        {
            GLuint program;
            GLuint viewProjUniform, tintUniform;
            GLuint vertFormat;
            GLuint vertBuffer;
            struct VertexType *bufferMap;
        } editorLine;

        struct
        {
            GLuint program;
            GLuint viewProjUniform, tintUniform, textureUniform, offsetUniform;
            GLuint vertFormat;
            GLuint vertBuffer, indBuffer;
            struct SectorVertexType *bufferMap;
            Index_t *indexMap;
            size_t highestVertIndex, highestIndIndex;
        } editorSector;
    } gl;

    struct
    {
        bool fetchingTextures;

        int gridSize;
        float zoomLevel;
        ImVec2 viewPosition;
        int selectionMode;

        int showSectorTextures;

#ifdef _DEBUG
        int mx, my, mtx, mty;
#endif

        pstring textureFilter;

        float realtimeFov;

        mat4 editorProjection;
        mat4 realtimeProjection;

        int startVertex;
        int lastVertForLine;
        size_t lineBuffer[LINE_BUFFER_CAP];
        size_t numLinesInBuffer;
    } data;

    struct
    {
        GLuint missingIcon;
        GLuint missingTexture;
    } defaultTextures;
};

const char* ColorIndexToString(enum Colors color);

void ResetSettings(struct EdSettings *settings);
bool LoadSettings(const char *settingsPath, struct EdSettings *settings);
void SaveSettings(const char *settingsPath, const struct EdSettings *settings);
void FreeSettings(struct EdSettings *settings);

bool LoadShaders(struct EdState *state);

bool InitEditor(struct EdState *state);
void DestroyEditor(struct EdState *state);

void ResizeEditorView(struct EdState *state, int width, int height);
void ResizeRealtimeView(struct EdState *state, int width, int height);

void RenderEditorView(struct EdState *state);
void RenderRealtimeView(struct EdState *state);
