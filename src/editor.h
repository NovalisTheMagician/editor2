#pragma once

#include "glad/gl.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

#include "vertex_types.h"

#include "vecmath.h"

#include "map.h"
#include "project.h"
#include "network.h"
#include "texture_collection.h"
#include "async_load.h"
#include "logging.h"
#include "script.h"
#include "vecmath.h"

#include <cglm/struct.h>

#define MAX_GAMEPATH_LEN 256
#define MAX_GAMEARGUMENTS_LEN 256

#define MAX_ZOOM 10.0f
#define MIN_ZOOM 0.05f

#define MAX_FOV 120
#define MIN_FOV 45

#define MAX_VERTEXPOINTSIZE 20
#define MIN_VERTEXPOINTSIZE 0.5f

#define TEXTURE_FILTER_LEN 256

#define EDIT_VERTEXBUFFER_CAP 4096
#define LINE_BUFFER_CAP 256

#define NUM_BUFFERS 3

typedef enum Theme
{
    THEME_IMGUI_LIGHT,
    THEME_IMGUI_DARK,
    THEME_IMGUI_CLASSIC,
    THEME_VALVE,
    THEME_DEUSEX,

    NUM_THEMES
} Theme;

typedef enum Colors
{
    COL_LOG_INFO,
    COL_LOG_WARN,
    COL_LOG_ERRO,
    COL_LOG_DEBU,

    COL_WORKSPACE_BACK,

    COL_BACKGROUND,
    COL_BACK_LINES,
    COL_BACK_MAJOR_LINES,

    COL_RTBACKGROUND,

    COL_VERTEX,
    COL_LINE,
    COL_SECTOR,

    COL_ACTIVE_EDIT,

    COL_VERTEX_HOVER,
    COL_VERTEX_SELECT,
    COL_LINE_HOVER,
    COL_LINE_SELECT,
    COL_SECTOR_HOVER,
    COL_SECTOR_SELECT,

    COL_LINE_INNER,

    NUM_COLORS
} Colors;

typedef enum SelectionMode
{
    MODE_VERTEX,
    MODE_LINE,
    MODE_SECTOR
} SelectionMode;

typedef enum EditState
{
    ESTATE_NORMAL,
    ESTATE_ADDVERTEX
} EditState;

typedef struct EdSettings
{
    Color colors[NUM_COLORS];
    int theme;

    bool showLineDir;

    float vertexPointSize;
    bool showGridLines, showMajorAxis;
    int realtimeFov;

    bool showFramerate, showFrametime;

    char gamePath[MAX_GAMEPATH_LEN];
    char launchArguments[MAX_GAMEARGUMENTS_LEN];
} EdSettings;

typedef struct BackgroundShaderData
{
    mat4s viewProj;
    Color tint;
    Color majorTint;
    float hOffset, vOffset, period;
    int majorIndex;
} BackgroundShaderData;

typedef struct EditorShaderData
{
    mat4s viewProj;
    Color tint;
    Vec2 coordOffset;
} EditorShaderData;

typedef struct EdState
{
    EdSettings settings;
    Map map;
    Project project;
    Network network;
    TextureCollection textures;
    AsyncJob async;
    LogBuffer log;
    Script script;

    struct
    {
        bool showMetrics;
        bool showAbout;

        bool showTextures;
        bool showEntities;
        bool showLogs;
        bool showStats;

        bool showBuildLog;

        bool showSettings;
        bool showMapSettings;
        bool showProjectSettings;

        bool showProperties;

        bool render3d;
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

        GLuint realtimeVertexFormat;

        GLuint backgroundLinesBuffer;

        GLuint whiteTexture;
        GLuint64 whiteTextureHandle;

        GLuint editorVertexFormat;
        GLuint editorVertexBuffer, editorIndexBuffer, editorShaderDataBuffer;
        EditorVertexType *editorVertexMap;
        Index_t *editorIndexMap;

        size_t editorMaxBufferCount;

        int currentBuffer;
        GLsync editorBufferFence[NUM_BUFFERS];
        size_t editorVertexBufferOffset[NUM_BUFFERS];
        size_t editorIndexBufferOffset[NUM_BUFFERS];

        struct
        {
            GLuint hProgram, vProgram;
            GLuint shaderDataBuffer;
            GLuint backVertexFormat;
        } editorBackProg;

        struct
        {
            GLuint program;
        } editorVertex;

        struct
        {
            GLuint program;
        } editorLine;

        struct
        {
            GLuint program;
        } editorSector;

        struct
        {
            GLuint program;
        } realtimeProgram;
    } gl;

    struct
    {
        int cachedDPI;

        bool fetchingTextures;
        bool autoScrollLogs;

        int gridSize;
        int altGridSize;
        float zoomLevel;
        ImVec2 viewPosition;
        int selectionMode;

        int showSectorTextures;

        uint64_t fetchStartTime, fetchEndTime;

#ifdef _DEBUG
        int mx, my, mtx, mty;
#endif

        char textureFilter[TEXTURE_FILTER_LEN];

        mat4s editorProjection;

        bool isDragging;
        Vec2 startDrag, endDrag;

        Vec2 editVertexBuffer[EDIT_VERTEXBUFFER_CAP], editVertexMouse, editDragMouse, editVertexDrag[3];
        size_t editVertexBufferSize;

        EditState editState;

        void **selectedElements;
        size_t numSelectedElements;
        void *hoveredElement;
    } data;

    struct {
        Vec3 cameraPosition, cameraDirection;
        mat4s realtimeProjection;
        float realtimeFov;
    } realtime;

    struct
    {
        GLuint missingIcon;
        GLuint64 missingIconHandle;
        GLuint missingTexture;
        GLuint64 missingTextureHandle;
        int missingTextureWidth, missingTextureHeight;
    } defaultTextures;
} EdState;

const char* ColorIndexToString(Colors color);

void ResetSettings(EdSettings *settings);
bool LoadSettings(const char *settingsPath, EdSettings *settings);
void SaveSettings(const char *settingsPath, const EdSettings *settings);
void FreeSettings(EdSettings *settings);

bool LoadShaders(EdState *state, char *error, size_t errorSize);

void ChangeMode(EdState *state, enum SelectionMode mode);

bool InitEditor(EdState *state, char *error, size_t errorSize);
void DestroyEditor(EdState *state);

void ResizeEditorView(EdState *state, int width, int height);
void ResizeRealtimeView(EdState *state, int width, int height);

void RenderEditorView(EdState *state);
void RenderRealtimeView(EdState *state);
