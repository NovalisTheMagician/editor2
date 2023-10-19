#pragma once

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>

#include "imgui_extensions.h"

#include "editor.h"
#include "common.h"

void AboutWindow(bool *p_open);
void SettingsWindow(bool *p_open, struct EdState *state);
void ToolbarWindow(bool *p_open, struct EdState *state);
void EditorWindow(bool *p_open, struct EdState *state);
void RealtimeWindow(bool *p_open, struct EdState *state);
void StatsWindow(bool *p_open, struct EdState *state);
void ProjectSettingsWindow(bool *p_open, struct EdState *state);
void MapSettingsWindow(bool *p_open, struct EdState *state);
void LogsWindow(bool *p_open, struct EdState *state);
struct Texture* TexturesWindow(bool *p_open, struct EdState *state, bool popup);

void PropertyWindow(bool *p_open, struct EdState *state);
