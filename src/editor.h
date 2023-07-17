#pragma once

#include "common.h"
#include "map.h"
#include <SDL2/SDL.h>

enum Colors
{
    BACKGROUND,
    BACK_LINES,

    VERTEX,
    LINE,
    SECTOR,

    NUM_COLORS
};

typedef float Color[4];

struct EdSettings
{
    uint16_t gridSize;
    Color colors[NUM_COLORS];

    char *gamePath;
    char *launchArguments;
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

        bool showSettings;
        bool showMapSettings;
    } ui;

    struct
    {
        int editorFramebufferWidth;
        int editorFramebufferHeight;
        GLuint editorFramebuffer;
        GLuint editorColorTexture;
        GLuint editorDepthTexture;
    } gl;
};

void ResetSettings(struct EdSettings *settings);
bool LoadSettings(const char *settingsPath, struct EdSettings *settings);
void SaveSettings(const char *settingsPath, const struct EdSettings *settings);

bool InitEditor(struct EdState *state);
void DestroyEditor(struct EdState *state);
void ResizeEditor(struct EdState *state, int width, int height);
bool HandleInputEvents(const SDL_Event *e, struct EdState *state, struct Map *map);
void RenderEditor(const struct EdState *state, const struct Map *map);
