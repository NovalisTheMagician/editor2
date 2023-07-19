#include "gui.h"

#include <tgmath.h>

void SetValveStyle(ImGuiStyle *style)
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
    colors[ImGuiCol_TabActive]                  = (ImVec4){0.59f, 0.54f, 0.18f, 1.00f};
    colors[ImGuiCol_TabUnfocused]               = (ImVec4){0.24f, 0.27f, 0.20f, 1.00f};
    colors[ImGuiCol_TabUnfocusedActive]         = (ImVec4){0.35f, 0.42f, 0.31f, 1.00f};
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
    style->WindowRounding = 0.0f;
    style->ChildRounding = 0.0f;
    style->FrameRounding = 0.0f;
    style->PopupRounding = 0.0f;
    style->ScrollbarRounding = 0.0f;
    style->GrabRounding = 0.0f;
    style->TabRounding = 0.0f;
}

void SetDeusExStyle(ImGuiStyle *style)
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
    colors[ImGuiCol_TabActive]              = (ImVec4){0.78f, 0.55f, 0.21f, 1.00f};
    colors[ImGuiCol_TabUnfocused]           = (ImVec4){0.07f, 0.10f, 0.15f, 0.97f};
    colors[ImGuiCol_TabUnfocusedActive]     = (ImVec4){0.14f, 0.26f, 0.42f, 1.00f};
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

    //style->WindowTitleAlign = (ImVec2){1.0f, 0.5f};
    //style->WindowMenuButtonPosition = ImGuiDir_Right;

    //style->DisplaySafeAreaPadding = (ImVec2){4, 4};
}

void SetStyle(enum Theme theme)
{
    switch(theme)
    {
    case THEME_IMGUI_LIGHT: igStyleColorsLight(igGetStyle()); break;
    case THEME_IMGUI_DARK: igStyleColorsDark(igGetStyle()); break;
    case THEME_IMGUI_CLASSIC: igStyleColorsClassic(igGetStyle()); break;
    case THEME_VALVE: SetValveStyle(igGetStyle()); break;
    case THEME_DEUSEX: SetDeusExStyle(igGetStyle()); break;
    default: igStyleColorsLight(igGetStyle()); break;
    }
}

static void AboutWindow(bool *p_open);
static void SettingsWindow(bool *p_open, struct EdState *state);
static void ToolbarWindow(bool *p_open, struct EdState *state);
static void EditorWindow(bool *p_open, struct EdState *state);
static void RealtimeWindow(bool *p_open, struct EdState *state);
static void MainMenuBar(bool *doQuit, struct EdState *state);

static void HandleShortcuts(struct EdState *state);

bool DoGui(struct EdState *state, struct Map *map)
{
    bool doQuit = false;

    MainMenuBar(&doQuit, state);

    if(state->ui.showAbout)
        AboutWindow(&state->ui.showAbout);

    if(state->ui.showMetrics)
        igShowMetricsWindow(&state->ui.showMetrics);

    if(state->ui.showToolbar)
        ToolbarWindow(&state->ui.showToolbar, state);

    if(state->ui.showSettings)
        SettingsWindow(&state->ui.showSettings, state);

    if(state->ui.show3dView)
        RealtimeWindow(&state->ui.show3dView, state);

    EditorWindow(NULL, state);

    HandleShortcuts(state);

    return doQuit;
}

static void MainMenuBar(bool *doQuit, struct EdState *state)
{
    if(igBeginMainMenuBar())
    {
        if(igBeginMenu("File", true))
        {
            if(igMenuItem_Bool("New", "Ctrl+N", false, true)) { printf("New Map!\n"); }
            if(igMenuItem_Bool("Open", "Ctrl+O", false, true)) { printf("Open Map!\n"); }
            if(igMenuItem_Bool("Save", "Ctrl+S", false, true)) { printf("Save Map!\n"); }
            if(igMenuItem_Bool("SaveAs", "", false, true)) { printf("Save Map As!\n"); }
            igSeparator();
            if(igMenuItem_Bool("Export", "Ctrl+E", false, true)) {  }
            igSeparator();
            if(igMenuItem_Bool("Quit", "Alt+F4", false, true)) { *doQuit = true; }
            igEndMenu();
        }

        if(igBeginMenu("Edit", true))
        {
            if(igMenuItem_Bool("Undo", "Ctrl+Z", false, true)) { printf("Undo!\n"); }
            if(igMenuItem_Bool("Redo", "Ctrl+Y", false, true)) { printf("Redo!\n"); }
            igSeparator();
            if(igMenuItem_Bool("Copy", "Ctrl+C", false, true)) { printf("Copy Menu!\n"); }
            if(igMenuItem_Bool("Paste", "Ctrl+V", false, true)) { printf("Paste!\n"); }
            if(igMenuItem_Bool("Cut", "Ctrl+X", false, true)) { printf("Cut!\n"); }
            igSeparator();
            if(igBeginMenu("Modes", true))
            {
                if(igMenuItem_Bool("Vertex", "", state->ui.selectionMode == MODE_VERTEX, true)) { state->ui.selectionMode = MODE_VERTEX; }
                if(igMenuItem_Bool("Line", "", state->ui.selectionMode == MODE_LINE, true)) { state->ui.selectionMode = MODE_LINE; }
                if(igMenuItem_Bool("Sector", "", state->ui.selectionMode == MODE_SECTOR, true)) { state->ui.selectionMode = MODE_SECTOR; }
                igEndMenu();
            }
            igSeparator();
            igMenuItem_BoolPtr("Map Settings", "", &state->ui.showMapSettings, true);
            igSeparator();
            igMenuItem_BoolPtr("Editor Options", "", &state->ui.showSettings, true);
            igEndMenu();
        }

        if(igBeginMenu("Tools", true))
        {
            igMenuItem_Bool("DRAGONS!", "", false, false);
            igEndMenu();
        }

        if(igBeginMenu("Build", true))
        {
            igMenuItem_Bool("DRAGONS!", "", false, false);
            igEndMenu();
        }

        if(igBeginMenu("Windows", true))
        {
            igMenuItem_BoolPtr("Toolbar", "", &state->ui.showToolbar, true);
            if(igMenuItem_Bool("Textures", "", false, true)) {  }
            if(igMenuItem_Bool("Entities", "", false, true)) {  }
            igMenuItem_BoolPtr("3D View", "Ctrl+W", &state->ui.show3dView, true);
            igSeparator();
            if(igMenuItem_Bool("Logs", "", false, true)) {  }
            igEndMenu();
        }

        if(igBeginMenu("Connect", true))
        {
            if(igMenuItem_Bool("Connect", "", false, true)) {  }
            if(igMenuItem_Bool("Disconnect", "", false, false)) {  }
            if(igMenuItem_Bool("Host", "", false, true)) {  }
            igSeparator();
            if(igMenuItem_Bool("Userlist", "", false, false)) {  }
            igEndMenu();
        }

        if(igBeginMenu("Help", true))
        {
            igMenuItem_BoolPtr("About", "", &state->ui.showAbout, true);
            igEndMenu();
        }

        char buffer[128];
        snprintf(buffer, sizeof buffer, "%d FPS (%.4f ms)", (int)round(igGetIO()->Framerate), 1.0f / igGetIO()->Framerate);
        ImVec2 textSize;
        igCalcTextSize(&textSize, buffer, buffer + strlen(buffer) + 1, false, 0);

        igSameLine(igGetWindowWidth() - textSize.x - 4, 0);
        igTextColored((ImVec4){ 0, 0.8f, 0.09f, 1 }, buffer);

        igEndMainMenuBar();
    }
}

static void AboutWindow(bool *p_open)
{
    if(igBegin("About", p_open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
    {
        igText("Editor2\nA map editor for the WeekRPG project\nMade by Novalis");
    }
    igEnd();
}

static void SettingsWindow(bool *p_open, struct EdState *state)
{
    //igSetNextWindowSize((ImVec2){ 0 }, ImGuiCond_FirstUseEver);
    if(igBegin("Options", p_open, 0))
    {
        if(igBeginTabBar("", 0))
        {
            if(igBeginTabItem("General", NULL, 0))
            {
                igInputText("Gamepath", state->settings.gamePath, sizeof state->settings.gamePath, 0, NULL, NULL);
                igInputText("Launch Arguments", state->settings.launchArguments, sizeof state->settings.launchArguments, 0, NULL, NULL);
                igSeparator();
                if(igButton("Reset Settings", (ImVec2){ 0, 0 })) { ResetSettings(&state->settings); }
                igEndTabItem();
            }

            if(igBeginTabItem("Colors", NULL, 0))
            {
                for(int i = 0; i < NUM_COLORS; ++i)
                {
                    igColorEdit4(ColorIndexToString(i), state->settings.colors[i], 0);
                }
                igEndTabItem();
            }

            if(igBeginTabItem("Appearance", NULL, 0))
            {
                static const char *themeElements[] = {"Imgui Light", "Imgui Dark", "Imgui Classic", "Valve", "Deus Ex"};
                static const size_t count = sizeof themeElements / sizeof *themeElements;
                if(igCombo_Str_arr("Theme", &state->settings.theme, themeElements, count, 5)) { SetStyle(state->settings.theme); }
                igEndTabItem();
            }

            igEndTabBar();
        }
    }
    igEnd();
}

static void ToolbarWindow(bool *p_open, struct EdState *state)
{
    if(igBegin("Toolbar", p_open, 0))
    {
        if(igButton("A", (ImVec2){ 24, 24 })) { igFocusWindow(NULL, 0); }
        igSameLine(0, 4);
        if(igButton("B", (ImVec2){ 24, 24 })) { igFocusWindow(NULL, 0); }
        igSameLine(0, 4);
        if(igButton("C", (ImVec2){ 24, 24 })) { igFocusWindow(NULL, 0); }
        igSameLine(0, 4);
        if(igButton("D", (ImVec2){ 24, 24 })) { igFocusWindow(NULL, 0); }
    }
    igEnd();
}

static void EditorWindow(bool *p_open, struct EdState *state)
{
    igSetNextWindowSize((ImVec2){ 800, 600 }, ImGuiCond_FirstUseEver);
    igSetNextWindowPos((ImVec2){ 40, 40 }, ImGuiCond_FirstUseEver, (ImVec2){ 0, 0 });

    if(igBegin("Editor", p_open, ImGuiWindowFlags_NoScrollbar))
    {
        igButton("A", (ImVec2){ 24, 24 });
        igSameLine(0, 4);
        igButton("B", (ImVec2){ 24, 24 });
        igSameLine(0, 4);
        igButton("C", (ImVec2){ 24, 24 });
        igSameLine(0, 4);
        igButton("D", (ImVec2){ 24, 24 });

        if(igBeginChild_ID(1000, (ImVec2){ 0, 0 }, false, ImGuiWindowFlags_NoMove))
        {
            ImVec2 clientArea;
            igGetContentRegionAvail(&clientArea);

            ImVec2 clientPos;
            igGetWindowPos(&clientPos);

            bool hovored = igIsWindowHovered(0);
            bool focused = igIsWindowFocused(0);

            if(igIsMouseClicked_Bool(ImGuiMouseButton_Left, false)) 
            {
                ImVec2 mpos;
                igGetMousePos(&mpos);
                int relX = (int)mpos.x - (int)clientPos.x;
                int relY = (int)mpos.y - (int)clientPos.y;
                
                if((relX >= 0 && relX < clientArea.x && relY >= 0 && relY < clientArea.y))
                    printf("Click: %d | %d; Hovered: %d; Focused: %d\n", relX,  relY, hovored, focused);
            }

            ResizeEditorView(state, clientArea.x, clientArea.y);
            igImage((void*)(intptr_t)state->gl.editorColorTexture, clientArea, (ImVec2){ 0, 0 }, (ImVec2){ 1, 1 }, (ImVec4){ 1, 1, 1, 1 }, (ImVec4){ 1, 1, 1, 0 });
        }
        igEndChild();
    }
    igEnd();
}

static void RealtimeWindow(bool *p_open, struct EdState *state)
{
    igSetNextWindowSize((ImVec2){ 800, 600 }, ImGuiCond_FirstUseEver);

    if(igBegin("3D View", p_open, ImGuiWindowFlags_NoScrollbar))
    {
        if(igBeginChild_ID(1000, (ImVec2){ 0, 0 }, false, ImGuiWindowFlags_NoMove))
        {
            ImVec2 clientArea;
            igGetContentRegionAvail(&clientArea);

            ImVec2 clientPos;
            igGetWindowPos(&clientPos);

            bool hovored = igIsWindowHovered(0);
            bool focused = igIsWindowFocused(0);

            if(igIsMouseClicked_Bool(ImGuiMouseButton_Left, false)) 
            {
                ImVec2 mpos;
                igGetMousePos(&mpos);
                int relX = (int)mpos.x - (int)clientPos.x;
                int relY = (int)mpos.y - (int)clientPos.y;
                
                if((relX >= 0 && relX < clientArea.x && relY >= 0 && relY < clientArea.y))
                    printf("Click: %d | %d; Hovered: %d; Focused: %d\n", relX,  relY, hovored, focused);
            }

            ResizeRealtimeView(state, clientArea.x, clientArea.y);
            igImage((void*)(intptr_t)state->gl.realtimeColorTexture, clientArea, (ImVec2){ 0, 0 }, (ImVec2){ 1, 1 }, (ImVec4){ 1, 1, 1, 1 }, (ImVec4){ 1, 1, 1, 0 });
        }
        igEndChild();
    }
    igEnd();
}

static void HandleShortcuts(struct EdState *state)
{
    if(igShortcut(ImGuiMod_Ctrl | ImGuiKey_N, 0, ImGuiInputFlags_RouteGlobalLow))
    {
        printf("New Map!\n");
    }

    if(igShortcut(ImGuiMod_Ctrl | ImGuiKey_O, 0, ImGuiInputFlags_RouteGlobalLow))
    {
        printf("Open Map!\n");
    }

    if(igShortcut(ImGuiMod_Ctrl | ImGuiKey_S, 0, ImGuiInputFlags_RouteGlobalLow))
    {
        printf("Save Map!\n");
    }

    if(igShortcut(ImGuiMod_Ctrl | ImGuiKey_E, 0, ImGuiInputFlags_RouteGlobalLow))
    {
        printf("Export Map!\n");
    }

    if(igShortcut(ImGuiMod_Ctrl | ImGuiKey_C, 0, ImGuiInputFlags_RouteGlobalLow))
    {
        printf("Copy!\n");
    }

    if(igShortcut(ImGuiMod_Ctrl | ImGuiKey_V, 0, ImGuiInputFlags_RouteGlobalLow))
    {
        printf("Paste!\n");
    }

    if(igShortcut(ImGuiMod_Ctrl | ImGuiKey_X, 0, ImGuiInputFlags_RouteGlobalLow))
    {
        printf("Cut!\n");
    }

    if(igShortcut(ImGuiMod_Ctrl | ImGuiKey_Z, 0, ImGuiInputFlags_RouteGlobalLow))
    {
        printf("Undo!\n");
    }

    if(igShortcut(ImGuiMod_Ctrl | ImGuiKey_Y, 0, ImGuiInputFlags_RouteGlobalLow))
    {
        printf("Redo!\n");
    }

    if(igShortcut(ImGuiMod_Ctrl | ImGuiKey_W, 0, ImGuiInputFlags_RouteGlobalLow))
    {
        state->ui.show3dView = !state->ui.show3dView;
    }
}
