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

    int editorFramebufferWidth;
    int editorFramebufferHeight;
    GLuint editorFramebuffer;
    GLuint editorColorTexture;
    GLuint editorDepthTexture;
};

void ResetSettings(struct EdSettings *settings);
bool LoadSettings(const char *settingsPath, struct EdSettings *settings);
void SaveSettings(const char *settingsPath, const struct EdSettings *settings);

bool InitEditor(struct EdSettings *settings);
void DestroyEditor(struct EdSettings *settings);
void ResizeEditor(struct EdSettings *settings, int width, int height);
bool HandleInputEvents(const SDL_Event *e, struct EdSettings *settings, struct Map *map);
void RenderEditor(const struct EdSettings *settings, const struct Map *map);
