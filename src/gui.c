#include "gui.h"

#include "edit.h"

#include <tgmath.h>
#include <assert.h>
#include <time.h>
#include <string.h>

#include "cimgui.h"
#include "ImGuiFileDialog.h"

#include "gwindows.h"
#include "dialogs.h"

#include "script.h"

static void SetValveStyle(ImGuiStyle *style)
{
    ImVec4* colors = style->Colors;
    colors[ImGuiCol_Text]                       = (ImVec4){1.00f, 1.00f, 1.00f, 1.00f};
    colors[ImGuiCol_TextDisabled]               = (ImVec4){0.50f, 0.50f, 0.50f, 1.00f};
    colors[ImGuiCol_WindowBg]                   = (ImVec4){0.29f, 0.34f, 0.26f, 1.00f};
    colors[ImGuiCol_ChildBg]                    = (ImVec4){0.29f, 0.34f, 0.26f, 1.00f};
    colors[ImGuiCol_PopupBg]                    = (ImVec4){0.24f, 0.27f, 0.20f, 1.00f};
    colors[ImGuiCol_Border]                     = (ImVec4){0.54f, 0.57f, 0.51f, 0.50f};
    colors[ImGuiCol_BorderShadow]               = (ImVec4){0.14f, 0.16f, 0.11f, 0.52f};
    colors[ImGuiCol_FrameBg]                    = (ImVec4){0.24f, 0.27f, 0.20f, 1.00f};
    colors[ImGuiCol_FrameBgHovered]             = (ImVec4){0.27f, 0.30f, 0.23f, 1.00f};
    colors[ImGuiCol_FrameBgActive]              = (ImVec4){0.30f, 0.34f, 0.26f, 1.00f};
    colors[ImGuiCol_TitleBg]                    = (ImVec4){0.24f, 0.27f, 0.20f, 1.00f};
    colors[ImGuiCol_TitleBgActive]              = (ImVec4){0.29f, 0.34f, 0.26f, 1.00f};
    colors[ImGuiCol_TitleBgCollapsed]           = (ImVec4){0.00f, 0.00f, 0.00f, 0.51f};
    colors[ImGuiCol_MenuBarBg]                  = (ImVec4){0.24f, 0.27f, 0.20f, 1.00f};
    colors[ImGuiCol_ScrollbarBg]                = (ImVec4){0.35f, 0.42f, 0.31f, 1.00f};
    colors[ImGuiCol_ScrollbarGrab]              = (ImVec4){0.28f, 0.32f, 0.24f, 1.00f};
    colors[ImGuiCol_ScrollbarGrabHovered]       = (ImVec4){0.25f, 0.30f, 0.22f, 1.00f};
    colors[ImGuiCol_ScrollbarGrabActive]        = (ImVec4){0.23f, 0.27f, 0.21f, 1.00f};
    colors[ImGuiCol_CheckMark]                  = (ImVec4){0.59f, 0.54f, 0.18f, 1.00f};
    colors[ImGuiCol_SliderGrab]                 = (ImVec4){0.35f, 0.42f, 0.31f, 1.00f};
    colors[ImGuiCol_SliderGrabActive]           = (ImVec4){0.54f, 0.57f, 0.51f, 0.50f};
    colors[ImGuiCol_Button]                     = (ImVec4){0.29f, 0.34f, 0.26f, 0.40f};
    colors[ImGuiCol_ButtonHovered]              = (ImVec4){0.35f, 0.42f, 0.31f, 1.00f};
    colors[ImGuiCol_ButtonActive]               = (ImVec4){0.54f, 0.57f, 0.51f, 0.50f};
    colors[ImGuiCol_Header]                     = (ImVec4){0.35f, 0.42f, 0.31f, 1.00f};
    colors[ImGuiCol_HeaderHovered]              = (ImVec4){0.35f, 0.42f, 0.31f, 0.6f};
    colors[ImGuiCol_HeaderActive]               = (ImVec4){0.54f, 0.57f, 0.51f, 0.50f};
    colors[ImGuiCol_Separator]                  = (ImVec4){0.14f, 0.16f, 0.11f, 1.00f};
    colors[ImGuiCol_SeparatorHovered]           = (ImVec4){0.54f, 0.57f, 0.51f, 1.00f};
    colors[ImGuiCol_SeparatorActive]            = (ImVec4){0.59f, 0.54f, 0.18f, 1.00f};
    colors[ImGuiCol_ResizeGrip]                 = (ImVec4){0.19f, 0.23f, 0.18f, 0.00f}; // grip invs
    colors[ImGuiCol_ResizeGripHovered]          = (ImVec4){0.54f, 0.57f, 0.51f, 1.00f};
    colors[ImGuiCol_ResizeGripActive]           = (ImVec4){0.59f, 0.54f, 0.18f, 1.00f};
    colors[ImGuiCol_Tab]                        = (ImVec4){0.35f, 0.42f, 0.31f, 1.00f};
    colors[ImGuiCol_TabHovered]                 = (ImVec4){0.54f, 0.57f, 0.51f, 0.78f};
    colors[ImGuiCol_TabSelected]                = (ImVec4){0.59f, 0.54f, 0.18f, 1.00f};
    colors[ImGuiCol_TabDimmed]                  = (ImVec4){0.24f, 0.27f, 0.20f, 1.00f};
    colors[ImGuiCol_TabDimmedSelected]          = (ImVec4){0.35f, 0.42f, 0.31f, 1.00f};
    colors[ImGuiCol_DockingPreview]             = (ImVec4){0.59f, 0.54f, 0.18f, 1.00f};
    colors[ImGuiCol_DockingEmptyBg]             = (ImVec4){0.20f, 0.20f, 0.20f, 1.00f};
    colors[ImGuiCol_PlotLines]                  = (ImVec4){0.61f, 0.61f, 0.61f, 1.00f};
    colors[ImGuiCol_PlotLinesHovered]           = (ImVec4){0.59f, 0.54f, 0.18f, 1.00f};
    colors[ImGuiCol_PlotHistogram]              = (ImVec4){1.00f, 0.78f, 0.28f, 1.00f};
    colors[ImGuiCol_PlotHistogramHovered]       = (ImVec4){1.00f, 0.60f, 0.00f, 1.00f};
    colors[ImGuiCol_TextSelectedBg]             = (ImVec4){0.59f, 0.54f, 0.18f, 1.00f};
    colors[ImGuiCol_DragDropTarget]             = (ImVec4){0.73f, 0.67f, 0.24f, 1.00f};
    colors[ImGuiCol_NavHighlight]               = (ImVec4){0.59f, 0.54f, 0.18f, 1.00f};
    colors[ImGuiCol_NavWindowingHighlight]      = (ImVec4){1.00f, 1.00f, 1.00f, 0.70f};
    colors[ImGuiCol_NavWindowingDimBg]          = (ImVec4){0.80f, 0.80f, 0.80f, 0.20f};
    colors[ImGuiCol_ModalWindowDimBg]           = (ImVec4){0.80f, 0.80f, 0.80f, 0.35f};

    style->FrameBorderSize = 1.0f;

    /*
    style->WindowRounding = 0.0f;
    style->ChildRounding = 0.0f;
    style->FrameRounding = 0.0f;
    style->PopupRounding = 0.0f;
    style->ScrollbarRounding = 0.0f;
    style->GrabRounding = 0.0f;
    style->TabRounding = 0.0f;
    */
}

static void SetDeusExStyle(ImGuiStyle *style)
{
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text]                   = (ImVec4){0.92f, 0.92f, 0.92f, 1.00f};
    colors[ImGuiCol_TextDisabled]           = (ImVec4){0.44f, 0.44f, 0.44f, 1.00f};
    colors[ImGuiCol_WindowBg]               = (ImVec4){0.06f, 0.06f, 0.06f, 1.00f};
    colors[ImGuiCol_ChildBg]                = (ImVec4){0.00f, 0.00f, 0.00f, 0.00f};
    colors[ImGuiCol_PopupBg]                = (ImVec4){0.08f, 0.08f, 0.08f, 0.94f};
    colors[ImGuiCol_Border]                 = (ImVec4){0.51f, 0.36f, 0.15f, 1.00f};
    colors[ImGuiCol_BorderShadow]           = (ImVec4){0.00f, 0.00f, 0.00f, 0.00f};
    colors[ImGuiCol_FrameBg]                = (ImVec4){0.11f, 0.11f, 0.11f, 1.00f};
    colors[ImGuiCol_FrameBgHovered]         = (ImVec4){0.51f, 0.36f, 0.15f, 1.00f};
    colors[ImGuiCol_FrameBgActive]          = (ImVec4){0.78f, 0.55f, 0.21f, 1.00f};
    colors[ImGuiCol_TitleBg]                = (ImVec4){0.51f, 0.36f, 0.15f, 1.00f};
    colors[ImGuiCol_TitleBgActive]          = (ImVec4){0.91f, 0.64f, 0.13f, 1.00f};
    colors[ImGuiCol_TitleBgCollapsed]       = (ImVec4){0.00f, 0.00f, 0.00f, 0.51f};
    colors[ImGuiCol_MenuBarBg]              = (ImVec4){0.11f, 0.11f, 0.11f, 1.00f};
    colors[ImGuiCol_ScrollbarBg]            = (ImVec4){0.06f, 0.06f, 0.06f, 0.53f};
    colors[ImGuiCol_ScrollbarGrab]          = (ImVec4){0.21f, 0.21f, 0.21f, 1.00f};
    colors[ImGuiCol_ScrollbarGrabHovered]   = (ImVec4){0.47f, 0.47f, 0.47f, 1.00f};
    colors[ImGuiCol_ScrollbarGrabActive]    = (ImVec4){0.81f, 0.83f, 0.81f, 1.00f};
    colors[ImGuiCol_CheckMark]              = (ImVec4){0.78f, 0.55f, 0.21f, 1.00f};
    colors[ImGuiCol_SliderGrab]             = (ImVec4){0.91f, 0.64f, 0.13f, 1.00f};
    colors[ImGuiCol_SliderGrabActive]       = (ImVec4){0.91f, 0.64f, 0.13f, 1.00f};
    colors[ImGuiCol_Button]                 = (ImVec4){0.51f, 0.36f, 0.15f, 1.00f};
    colors[ImGuiCol_ButtonHovered]          = (ImVec4){0.91f, 0.64f, 0.13f, 1.00f};
    colors[ImGuiCol_ButtonActive]           = (ImVec4){0.78f, 0.55f, 0.21f, 1.00f};
    colors[ImGuiCol_Header]                 = (ImVec4){0.51f, 0.36f, 0.15f, 1.00f};
    colors[ImGuiCol_HeaderHovered]          = (ImVec4){0.91f, 0.64f, 0.13f, 1.00f};
    colors[ImGuiCol_HeaderActive]           = (ImVec4){0.93f, 0.65f, 0.14f, 1.00f};
    colors[ImGuiCol_Separator]              = (ImVec4){0.21f, 0.21f, 0.21f, 1.00f};
    colors[ImGuiCol_SeparatorHovered]       = (ImVec4){0.91f, 0.64f, 0.13f, 1.00f};
    colors[ImGuiCol_SeparatorActive]        = (ImVec4){0.78f, 0.55f, 0.21f, 1.00f};
    colors[ImGuiCol_ResizeGrip]             = (ImVec4){0.21f, 0.21f, 0.21f, 1.00f};
    colors[ImGuiCol_ResizeGripHovered]      = (ImVec4){0.91f, 0.64f, 0.13f, 1.00f};
    colors[ImGuiCol_ResizeGripActive]       = (ImVec4){0.78f, 0.55f, 0.21f, 1.00f};
    colors[ImGuiCol_Tab]                    = (ImVec4){0.51f, 0.36f, 0.15f, 1.00f};
    colors[ImGuiCol_TabHovered]             = (ImVec4){0.91f, 0.64f, 0.13f, 1.00f};
    colors[ImGuiCol_TabSelected]            = (ImVec4){0.78f, 0.55f, 0.21f, 1.00f};
    colors[ImGuiCol_TabDimmed]              = (ImVec4){0.07f, 0.10f, 0.15f, 0.97f};
    colors[ImGuiCol_TabDimmedSelected]      = (ImVec4){0.14f, 0.26f, 0.42f, 1.00f};
    colors[ImGuiCol_PlotLines]              = (ImVec4){0.61f, 0.61f, 0.61f, 1.00f};
    colors[ImGuiCol_PlotLinesHovered]       = (ImVec4){1.00f, 0.43f, 0.35f, 1.00f};
    colors[ImGuiCol_PlotHistogram]          = (ImVec4){0.90f, 0.70f, 0.00f, 1.00f};
    colors[ImGuiCol_PlotHistogramHovered]   = (ImVec4){1.00f, 0.60f, 0.00f, 1.00f};
    colors[ImGuiCol_TextSelectedBg]         = (ImVec4){0.26f, 0.59f, 0.98f, 0.35f};
    colors[ImGuiCol_DragDropTarget]         = (ImVec4){1.00f, 1.00f, 0.00f, 0.90f};
    colors[ImGuiCol_NavHighlight]           = (ImVec4){0.26f, 0.59f, 0.98f, 1.00f};
    colors[ImGuiCol_NavWindowingHighlight]  = (ImVec4){1.00f, 1.00f, 1.00f, 0.70f};
    colors[ImGuiCol_NavWindowingDimBg]      = (ImVec4){0.80f, 0.80f, 0.80f, 0.20f};
    colors[ImGuiCol_ModalWindowDimBg]       = (ImVec4){0.80f, 0.80f, 0.80f, 0.35f};

    /*
    style->FramePadding = (ImVec2){4, 2};
    style->ItemSpacing = (ImVec2){10, 2};
    style->IndentSpacing = 12;
    style->ScrollbarSize = 10;

    style->WindowRounding = 4;
    style->FrameRounding = 4;
    style->PopupRounding = 4;
    style->ScrollbarRounding = 6;
    style->GrabRounding = 4;
    style->TabRounding = 4;
    */

    //style->WindowTitleAlign = (ImVec2){1.0f, 0.5f};
    //style->WindowMenuButtonPosition = ImGuiDir_Right;

    //style->DisplaySafeAreaPadding = (ImVec2){4, 4};
}

void SetStyle(enum Theme theme)
{
    ImGuiStyle *style = igGetStyle();
    style->FrameBorderSize = 0.0f;
    switch(theme)
    {
    case THEME_IMGUI_LIGHT: igStyleColorsLight(style); break;
    case THEME_IMGUI_DARK: igStyleColorsDark(style); break;
    case THEME_IMGUI_CLASSIC: igStyleColorsClassic(style); break;
    case THEME_VALVE: SetValveStyle(style); break;
    case THEME_DEUSEX: SetDeusExStyle(style); break;
    default: igStyleColorsLight(style); break;
    }
}

static void MainMenuBar(bool *doQuit, EdState *state);
static void DockSpace(bool *doQuit, EdState *state);
static void FileDialog(bool *doQuit);

static void ProjectSavePopup(EdState *state, bool *quitRequest);
static void MapSavePopup(EdState *state, bool *quitRequest);

static void HandleShortcuts(EdState *state);

static void DoNewMap(EdState *state);
static void DoLoadMap(EdState *state);

ImGuiFileDialog *cfileDialog;

static bool openProjectPopup = false;
static bool openMapPopup = false;
static enum SaveModalAction modalAction = SMA_NEW;

void InitGui(void)
{
    cfileDialog = IGFD_Create();
}

void FreeGui(void)
{
    IGFD_Destroy(cfileDialog);
}

bool DoGui(EdState *state, bool doQuit)
{
    openProjectPopup = false;
    openMapPopup = false;

    DockSpace(&doQuit, state);

    if(state->ui.showAbout)
        AboutWindow(&state->ui.showAbout);

    if(state->ui.showMetrics)
        igShowMetricsWindow(&state->ui.showMetrics);

    if(state->ui.showSettings)
        SettingsWindow(&state->ui.showSettings, state);

    if(state->ui.showStats)
        StatsWindow(&state->ui.showStats, state);

    if(state->ui.showProjectSettings)
        ProjectSettingsWindow(&state->ui.showProjectSettings, state);

    if(state->ui.showMapSettings)
        MapSettingsWindow(&state->ui.showMapSettings, state);

    if(state->ui.showTextures)
        TexturesWindow(&state->ui.showTextures, state);

    if(state->ui.showLogs)
        LogsWindow(&state->ui.showLogs, state);

    if(state->ui.showProperties)
        PropertyWindow(&state->ui.showProperties, state);

    if(state->ui.showEntities)
        EntitiesWindow(&state->ui.showEntities, state);

    EditorWindow(NULL, state);
    RealtimeWindow(NULL, state);

    if(doQuit)
    {
        if(state->map.dirty)
        {
            doQuit = false; // dont quit yet

            openMapPopup = true;
            modalAction = SMA_QUIT;
        }

        if(state->project.dirty)
        {
            doQuit = false; // dont quit yet

            openProjectPopup = true;
            modalAction = SMA_QUIT;
        }
    }

    if(openProjectPopup)
        igOpenPopup_Str("Save Project?", 0);

    if(openMapPopup)
        igOpenPopup_Str("Save Map?", 0);

    ProjectSavePopup(state, &doQuit);
    MapSavePopup(state, &doQuit);

    HandleShortcuts(state);

    FileDialog(&doQuit);

    static bool firstTime = true;
    if(firstTime)
    {
        firstTime = false;
        igSetWindowFocus_Str("Editor");
    }

    return doQuit;
}

static void MainMenuBar(bool *doQuit, EdState *state)
{
    bool allowFileOps = state->network.hosting || !state->network.connected;
    if(igBeginMainMenuBar())
    {
        if(igBeginMenu("File", true))
        {
            if(igMenuItem_Bool("New Project", "", false, allowFileOps)) { if(state->project.dirty) { openProjectPopup = true; modalAction = SMA_NEW; } else NewProject(&state->project); }
            if(igMenuItem_Bool("Open Project", "", false, allowFileOps)) { if(state->project.dirty) { openProjectPopup = true; modalAction = SMA_OPEN; } else OpenProjectDialog(&state->project); }
            if(igMenuItem_Bool("Save Project", "", false, state->project.dirty && allowFileOps))
            {
                if(string_length(state->project.file) == 0)
                    SaveProjectDialog(&state->project, false);
                else
                    SaveProject(&state->project);
            }
            igSeparator();
            if(igMenuItem_Bool("New Map", "Ctrl+N", false, allowFileOps)) { if(state->map.dirty) { openMapPopup = true; modalAction = SMA_NEW; } else DoNewMap(state); }
            if(igMenuItem_Bool("Open Map", "Ctrl+O", false, allowFileOps)) { if(state->map.dirty) { openMapPopup = true; modalAction = SMA_OPEN; } else DoLoadMap(state); }
            if(igMenuItem_Bool("Save Map", "Ctrl+S", false, state->map.dirty && allowFileOps))
            {
                if(string_length(state->map.file) == 0)
                    SaveMapDialog(&state->map, false);
                else
                    SaveMap(&state->map);
            }
            if(igMenuItem_Bool("SaveAs Map", "", false, allowFileOps)) { SaveMapDialog(&state->map, doQuit); }
            igSeparator();
            if(igMenuItem_Bool("Quit", "Alt+F4", false, true)) { *doQuit = true; }
            igEndMenu();
        }

        if(igBeginMenu("Edit", true))
        {
            if(igMenuItem_Bool("Undo", "Ctrl+Z", false, true)) { LogInfo("Undo!"); }
            if(igMenuItem_Bool("Redo", "Ctrl+Y", false, true)) { LogInfo("Redo!"); }
            igSeparator();
            if(igMenuItem_Bool("Copy", "Ctrl+C", false, true)) { EditCopy(state); }
            if(igMenuItem_Bool("Paste", "Ctrl+V", false, true)) { EditPaste(state); }
            if(igMenuItem_Bool("Cut", "Ctrl+X", false, true)) { EditCut(state); }
            igSeparator();
            if(igBeginMenu("Modes", true))
            {
                if(igMenuItem_Bool("Vertex", "V", state->data.selectionMode == MODE_VERTEX, true)) { ChangeMode(state, MODE_VERTEX); }
                if(igMenuItem_Bool("Line", "L", state->data.selectionMode == MODE_LINE, true)) { ChangeMode(state, MODE_LINE); }
                if(igMenuItem_Bool("Sector", "S", state->data.selectionMode == MODE_SECTOR, true)) { ChangeMode(state, MODE_SECTOR); }
                igEndMenu();
            }
            igSeparator();
            //igMenuItem_BoolPtr("Map Settings", "", &state->ui.showMapSettings, true);
            igMenuItem_BoolPtr("Project Settings", "", &state->ui.showProjectSettings, true);
            igSeparator();
            igMenuItem_BoolPtr("Editor Options", "", &state->ui.showSettings, true);
            igEndMenu();
        }

        if(igBeginMenu("Tools", true))
        {
            for(size_t i = 0; i < state->script.numPlugins; ++i)
            {
                bool enabled = state->script.plugins[i].flags & Plugin_HasPrerequisite ? ScriptPluginCheck(&state->script, i) : true;
                if(igMenuItem_Bool(state->script.plugins[i].name, "", false, enabled))
                    ScriptPluginExec(&state->script, i);
            }
            if(state->script.numPlugins == 0)
            {
                igMenuItem_Bool("No Plugins Registered", "", false, false);
            }
            igEndMenu();
        }

        if(igBeginMenu("Build", true))
        {
            if(igMenuItem_Bool("Map", "Ctrl+B", false, true)) {  }
            if(igBeginMenu("Make", true))
            {
                if(igMenuItem_Bool("Textures", "", false, true)) {  }
                if(igMenuItem_Bool("Maps", "", false, true)) {  }
                igSeparator();
                if(igMenuItem_Bool("All", "Ctrl+M", false, true)) {  }
                if(igMenuItem_Bool("Clean", "", false, true)) {  }
                igEndMenu();
            }
            if(igMenuItem_Bool("Run Map", "Ctrl+R", false, true)) {  }
            igSeparator();
            igMenuItem_BoolPtr("Build Log", "", &state->ui.showBuildLog, true);
            igEndMenu();
        }

        if(igBeginMenu("Windows", true))
        {
            igMenuItem_BoolPtr("Textures", "Ctrl+T", &state->ui.showTextures, true);
            igMenuItem_BoolPtr("Entities", "Ctrl+E", &state->ui.showEntities, true);
            igMenuItem_BoolPtr("Properties", "Ctrl+P", &state->ui.showProperties, true);
            igSeparator();
            igMenuItem_BoolPtr("Logs", "Ctrl+L", &state->ui.showLogs, true);
#ifdef _DEBUG
            igSeparator();
            igMenuItem_BoolPtr("Debug", "", &state->ui.showStats, true);
#endif
            igEndMenu();
        }

        if(igBeginMenu("Connect", true))
        {
            if(igMenuItem_Bool("Connect", "", false, !state->network.connected)) { state->network.connected = true; }
            if(igMenuItem_Bool("Disconnect", "", false, state->network.connected)) { state->network.connected = false; state->network.hosting = false; }
            if(igMenuItem_Bool("Host", "", false, !state->network.connected)) { state->network.hosting = true; state->network.connected = true; }
            igSeparator();
            if(igMenuItem_Bool("Userlist", "", false, state->network.connected)) {  }
            igEndMenu();
        }

        if(igBeginMenu("Help", true))
        {
            igMenuItem_BoolPtr("About", "", &state->ui.showAbout, true);
            igEndMenu();
        }

        char buffer[64];
        float framerate = igGetIO()->Framerate;
        snprintf(buffer, sizeof buffer, "%d FPS (%.4f ms)", (int)round(framerate), 1.0f / framerate);
        ImVec2 textSize;
        igCalcTextSize(&textSize, buffer, buffer + strlen(buffer) + 1, false, 0);

        igSameLine(igGetWindowWidth() - textSize.x - 4, 0);
        igTextColored((ImVec4){ 0, 0.8f, 0.09f, 1 }, buffer);

        if(state->data.fetchingTextures)
        {
            const char fetchText[] = "Fetching Textures...";
            float prevSize = textSize.x;
            igCalcTextSize(&textSize, fetchText, NULL, false, 0);
            igSameLine(igGetWindowWidth() - (prevSize + textSize.x + 16), 0);
            igTextColored((ImVec4){ 226 / 255.0f, 99 / 255.0f, 16 / 255.0f, 1 }, fetchText);
        }

        igEndMainMenuBar();
    }
}

static void ProjectSavePopup(EdState *state, bool *quitRequest)
{
    if(igBeginPopupModal("Save Project?", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
    {
        igText("You have unsaved changes to the Project.\nDo you want to save them?");
        if(igButton("Yes", (ImVec2){ 64, 0 }))
        {
            if(string_length(state->project.file) > 0)
                SaveProject(&state->project);

            switch(modalAction)
            {
            case SMA_NEW: NewProject(&state->project); break;
            case SMA_OPEN: OpenProjectDialog(&state->project); break;
            case SMA_QUIT:
            {
                SaveProjectDialog(&state->project, true);
                break;
            }
            }
            igCloseCurrentPopup();
        }
        igSameLine(0, 4);
        if(igButton("No", (ImVec2){ 64, 0 }))
        {
            switch(modalAction)
            {
            case SMA_NEW: NewProject(&state->project); break;
            case SMA_OPEN: OpenProjectDialog(&state->project); break;
            case SMA_QUIT: *quitRequest = true; break;
            }
            igCloseCurrentPopup();
        }
        igSameLine(0, 4);
        if(igButton("Cancel", (ImVec2){ 64, 0 }))
        {
            igCloseCurrentPopup();
        }
        igEndPopup();
    }
}

static void MapSavePopup(EdState *state, bool *quitRequest)
{
    if(igBeginPopupModal("Save Map?", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
    {
        igText("You have unsaved changes to the Map.\nDo you want to save them?");
        if(igButton("Yes", (ImVec2){ 64, 0 }))
        {
            if(string_length(state->map.file) > 0)
                SaveMap(&state->map);

            switch(modalAction)
            {
            case SMA_NEW: DoNewMap(state); break;
            case SMA_OPEN: DoLoadMap(state); break;
            case SMA_QUIT:
            {
                SaveMapDialog(&state->map, true);
                break;
            }
            }
            igCloseCurrentPopup();
        }
        igSameLine(0, 4);
        if(igButton("No", (ImVec2){ 64, 0 }))
        {
            switch(modalAction)
            {
            case SMA_NEW: DoNewMap(state); break;
            case SMA_OPEN: DoLoadMap(state); break;
            case SMA_QUIT: *quitRequest = true; break;
            }
            igCloseCurrentPopup();
        }
        igSameLine(0, 4);
        if(igButton("Cancel", (ImVec2){ 64, 0 }))
        {
            if(modalAction == SMA_QUIT) *quitRequest = false;
            igCloseCurrentPopup();
        }
        igEndPopup();
    }
}

static void FileDialog(bool *doQuit)
{
    ImGuiIO* ioptr = igGetIO();
    ImVec2 maxSize;
    maxSize.x = ioptr->DisplaySize.x * 0.8f;
    maxSize.y = ioptr->DisplaySize.y * 0.8f;
    ImVec2 minSize;
    minSize.x = maxSize.x * 0.5f;
    minSize.y = maxSize.y * 0.5f;
    if (IGFD_DisplayDialog(cfileDialog, "filedlg", ImGuiWindowFlags_NoCollapse, minSize, maxSize))
    {
        FileDialogAction *action = IGFD_GetUserDatas(cfileDialog);
        assert(action);
        if(IGFD_IsOk(cfileDialog))
        {
            char *cfilePathName = IGFD_GetFilePathName(cfileDialog, IGFD_ResultMode_AddIfNoFileExt);
            if(!cfilePathName)
                cfilePathName = IGFD_GetCurrentPath(cfileDialog);
            action->callback(cfilePathName, action->data);
            if (cfilePathName)
            {
                free(cfilePathName);
            }
            *doQuit = action->quitRequest;
        }

        free(action);
        IGFD_CloseDialog(cfileDialog);
    }
}

static void HandleShortcuts(EdState *state)
{
    if(igShortcut_Nil(ImGuiMod_Ctrl | ImGuiKey_N, ImGuiInputFlags_RouteGlobal))
    {
        if(state->map.dirty) { openMapPopup = true; modalAction = SMA_NEW; }
        else DoNewMap(state);
    }

    if(igShortcut_Nil(ImGuiMod_Ctrl | ImGuiKey_O, ImGuiInputFlags_RouteGlobal))
    {
        if(state->map.dirty) { openMapPopup = true; modalAction = SMA_OPEN; }
        else DoLoadMap(state);
    }

    if(igShortcut_Nil(ImGuiMod_Ctrl | ImGuiKey_S, ImGuiInputFlags_RouteGlobal))
    {
        if(state->map.dirty)
        {
            if(string_length(state->map.file) == 0)
                SaveMapDialog(&state->map, false);
            else
                SaveMap(&state->map);
        }
    }

    if(igShortcut_Nil(ImGuiMod_Ctrl | ImGuiKey_W, ImGuiInputFlags_RouteGlobal))
    {
        const char *window = state->ui.render3d ? "Editor" : "3D View";
        igSetWindowFocus_Str(window);
    }

    if(igShortcut_Nil(ImGuiMod_Ctrl | ImGuiKey_E, ImGuiInputFlags_RouteGlobal))
    {
        state->ui.showEntities = !state->ui.showEntities;
    }

    if(igShortcut_Nil(ImGuiMod_Ctrl | ImGuiKey_T, ImGuiInputFlags_RouteGlobal))
    {
        state->ui.showTextures = !state->ui.showTextures;
    }

    if(igShortcut_Nil(ImGuiMod_Ctrl | ImGuiKey_L, ImGuiInputFlags_RouteGlobal))
    {
        state->ui.showLogs = !state->ui.showLogs;
    }
}

static void DoNewMap(EdState *state)
{
    state->data.editVertexBufferSize = 0;
    state->data.numSelectedElements = 0;
    NewMap(&state->map);
}

static void DoLoadMap(EdState *state)
{
    state->data.editVertexBufferSize = 0;
    state->data.numSelectedElements = 0;
    OpenMapDialog(&state->map);
}

static void DockSpace(bool *doQuit, EdState *state)
{
    ImGuiViewport *viewport = igGetMainViewport();

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    igSetNextWindowPos(viewport->Pos, ImGuiCond_None, (ImVec2){ 0, 0 });
    igSetNextWindowSize(viewport->Size, ImGuiCond_None);
    igSetNextWindowViewport(viewport->ID);
    igPushStyleVar_Float(ImGuiStyleVar_WindowRounding, 0.0f);
    igPushStyleVar_Float(ImGuiStyleVar_WindowBorderSize, 0.0f);
    igPushStyleVar_Vec2(ImGuiStyleVar_WindowPadding, (ImVec2){ 0, 0 });
    igBegin("Dockspacewindow", NULL, windowFlags);
    igPopStyleVar(3);

    if(igDockBuilderGetNode(igGetID_Str("Dockspace")) == NULL)
    {
        ImGuiID dockSpaceId = igGetID_Str("Dockspace");

        igDockBuilderRemoveNode(dockSpaceId);
        igDockBuilderAddNode(dockSpaceId, 0);
        igDockBuilderSetNodeSize(dockSpaceId, viewport->Size);

        ImGuiID dockMainId = dockSpaceId;

        ImGuiID dock1 = igDockBuilderSplitNode(dockMainId, ImGuiDir_Left, 0.7f, NULL, &dockMainId);
        ImGuiID dock2 = igDockBuilderSplitNode(dockMainId, ImGuiDir_Right, 0.3f, NULL, &dockMainId);
        ImGuiID dock3 = igDockBuilderSplitNode(dock2, ImGuiDir_Down, 0.5f, NULL, &dock2);
        ImGuiID dock4 = igDockBuilderSplitNode(dock1, ImGuiDir_Down, 0.3f, NULL, &dock1);
        ImGuiID dock5 = igDockBuilderSplitNode(dock4, ImGuiDir_Right, 0.4f, NULL, &dock4);

        igDockBuilderDockWindow("Editor", dock1);
        igDockBuilderDockWindow("3D View", dock1);
        igDockBuilderDockWindow("Texture Browser", dock2);
        igDockBuilderDockWindow("Entities Browser", dock2);
        igDockBuilderDockWindow("Logs", dock4);
        igDockBuilderDockWindow("Properties", dock3);
        igDockBuilderDockWindow("Debug", dock5);

        igDockBuilderFinish(dockSpaceId);
    }

    igPushStyleColor_Vec4(ImGuiCol_DockingEmptyBg, (ImVec4){ .y = 1 });
    ImGuiID dockSpaceId = igGetID_Str("Dockspace");
    igDockSpace(dockSpaceId, (ImVec2){ 0, 0 }, 0, NULL);
    igPopStyleColor(1);

    MainMenuBar(doQuit, state);

    igEnd();
}
