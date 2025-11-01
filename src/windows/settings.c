#include "../gwindows.h"

#include "cimgui.h"
#include "logging.h"
#include "utils.h"
#include "../gui.h"

void SettingsWindow(bool *p_open, EdState *state)
{
    igSetNextWindowSize((ImVec2){ 600, 300 }, ImGuiCond_FirstUseEver);
    if(igBegin("Options", p_open, ImGuiWindowFlags_NoDocking))
    {
        if(igBeginTabBar("", 0))
        {
            if(igBeginTabItem("General", NULL, 0))
            {
                igSeparatorText("Game");
                igInputText("Gamepath", state->settings.gamePath, sizeof state->settings.gamePath, 0, NULL, NULL);
                igInputText("Launch Arguments", state->settings.launchArguments, sizeof state->settings.launchArguments, 0, NULL, NULL);
                igSeparatorText("Other");
                if(igInputInt("Logs Scrollback Size", &state->log.capacity, 1, 10, 0))
                {
                    if(state->log.capacity < 4) state->log.capacity = 4;
                    LogResizeBuffer(&state->log);
                }
                igCheckbox("Show Framerate", &state->settings.showFramerate);
                igCheckbox("Show Frametime", &state->settings.showFrametime);
                igSeparator();
                if(igButton("Reset Settings", (ImVec2){ 0, 0 })) { ResetSettings(&state->settings); }
                igEndTabItem();
            }

            if(igBeginTabItem("Editor", NULL, 0))
            {
                igDragFloat("Vertex Point Size", &state->settings.vertexPointSize, 0.1f, MIN_VERTEXPOINTSIZE, MAX_VERTEXPOINTSIZE, "%.1f", 0);
                igCheckbox("Show Grid", &state->settings.showGridLines);
                igCheckbox("Show Major Axis", &state->settings.showMajorAxis);
                igCheckbox("Show Line direction", &state->settings.showLineDir);
                igEndTabItem();
            }

            if(igBeginTabItem("3D View", NULL, 0))
            {
                igSliderInt("Field of View", &state->settings.realtimeFov, MIN_FOV, MAX_FOV, "%d°", 0);
                igEndTabItem();
            }

            if(igBeginTabItem("Colors", NULL, 0))
            {
                for(int i = 0; i < NUM_COLORS; ++i)
                {
                    igColorEdit4(ColorIndexToString(i), (float*)&state->settings.colors[i], 0);
                }
                igEndTabItem();
            }

            if(igBeginTabItem("Appearance", NULL, 0))
            {
                static const char *themeElements[] = {"Imgui Light", "Imgui Dark", "Imgui Classic", "Valve old VGUI", "Deus Ex Human Barbecue"};
                static const size_t count = COUNT_OF(themeElements);
                if(igCombo_Str_arr("Theme", &state->settings.theme, themeElements, count, 5)) { SetStyle(state->settings.theme); }
                igEndTabItem();
            }

            igEndTabBar();
        }
    }
    igEnd();
}
