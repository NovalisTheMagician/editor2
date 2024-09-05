#pragma once

#include "editor.h"

void AboutWindow(bool *p_open);
void SettingsWindow(bool *p_open, EdState *state);
void ToolbarWindow(bool *p_open, EdState *state);
void EditorWindow(bool *p_open, EdState *state);
void RealtimeWindow(bool *p_open, EdState *state);
void StatsWindow(bool *p_open, EdState *state);
void ProjectSettingsWindow(bool *p_open, EdState *state);
void MapSettingsWindow(bool *p_open, EdState *state);
void LogsWindow(bool *p_open, EdState *state);
void TexturesWindow(bool *p_open, EdState *state);

void PropertyWindow(bool *p_open, EdState *state);
