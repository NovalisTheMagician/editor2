#pragma once

#include "common.h"
#include "map.h"
#include "project.h"
#include "network.h"
#include "texture_collection.h"
#include "async_load.h"
#include "logging.h"

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

typedef float Color[4];
static inline void SetColor(Color *to, Color from)
{
    (*to)[0] = from[0];
    (*to)[1] = from[1];
    (*to)[2] = from[2];
    (*to)[3] = from[3];
}

#define Col2Vec4(col) ((vec4s){ .r = col[0], .g = col[1], .b = col[2], .a = col[3] })

typedef struct EdSettings
{
    Color colors[NUM_COLORS];
    int theme;

    float vertexPointSize;
    bool showGridLines, showMajorAxis;
    int realtimeFov;

    pstring gamePath;
    pstring launchArguments;
} EdSettings;

typedef struct BackgroundShaderData
{
    mat4s viewProj;
    vec4s tint;
    vec4s majorTint;
    float hOffset, vOffset, period;
    int majorIndex;
} BackgroundShaderData;

typedef struct EditorShaderData
{
    mat4s viewProj;
    vec4s tint;
    vec2s coordOffset;
} EditorShaderData;

typedef struct EditorVertexType
{
    vec2s position;
    vec4s color;
    vec2s texCoord;
} EditorVertexType;

typedef uint32_t Index_t;

typedef struct EdState
{
    EdSettings settings;
    Map map;
    Project project;
    Network network;
    TextureCollection textures;
    AsyncJob async;
    LogBuffer log;

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

        bool showProperties;
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
    } gl;

    struct
    {
        bool fetchingTextures;
        bool autoScrollLogs;

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

        mat4s editorProjection;
        mat4s realtimeProjection;

        bool isDragging;
        vec2s startDrag, endDrag;

        vec2s editVertexBuffer[EDIT_VERTEXBUFFER_CAP], editVertexMouse, editDragMouse, editVertexDrag[3];
        size_t editVertexBufferSize;

        EditState editState;

        void **selectedElements;
        size_t numSelectedElements;
        void *hoveredElement;
    } data;

    struct
    {
        GLuint missingIcon;
        GLuint missingTexture;
    } defaultTextures;
} EdState;

const char* ColorIndexToString(Colors color);

void ResetSettings(EdSettings *settings);
bool LoadSettings(const char *settingsPath, EdSettings *settings);
void SaveSettings(const char *settingsPath, const EdSettings *settings);
void FreeSettings(EdSettings *settings);

bool LoadShaders(EdState *state);

void ChangeMode(EdState *state, enum SelectionMode mode);

bool InitEditor(EdState *state);
void DestroyEditor(EdState *state);

void ResizeEditorView(EdState *state, int width, int height);
void ResizeRealtimeView(EdState *state, int width, int height);

void RenderEditorView(EdState *state);
void RenderRealtimeView(EdState *state);
