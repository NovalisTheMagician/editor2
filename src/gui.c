#include "gui.h"

void SetStyle(ImGuiStyle *style)
{
    style->Colors[ImGuiCol_Text]                  = (ImVec4){0.00f, 0.00f, 0.00f, 1.00f};
    style->Colors[ImGuiCol_TextDisabled]          = (ImVec4){0.60f, 0.60f, 0.60f, 1.00f};
    //style->Colors[ImGuiCol_TextHovered]           = (ImVec4){1.00f, 1.00f, 1.00f, 1.00f};
    //style->Colors[ImGuiCol_TextActive]            = (ImVec4){1.00f, 1.00f, 0.00f, 1.00f};
    style->Colors[ImGuiCol_WindowBg]              = (ImVec4){0.94f, 0.94f, 0.94f, 1.00f};
    //style->Colors[ImGuiCol_ChildWindowBg]         = (ImVec4){0.00f, 0.00f, 0.00f, 0.00f};
    style->Colors[ImGuiCol_Border]                = (ImVec4){0.00f, 0.00f, 0.00f, 0.39f};
    style->Colors[ImGuiCol_BorderShadow]          = (ImVec4){1.00f, 1.00f, 1.00f, 0.10f};
    style->Colors[ImGuiCol_FrameBg]               = (ImVec4){1.00f, 1.00f, 1.00f, 1.00f};
    style->Colors[ImGuiCol_FrameBgHovered]        = (ImVec4){0.26f, 0.59f, 0.98f, 0.40f};
    style->Colors[ImGuiCol_FrameBgActive]         = (ImVec4){0.26f, 0.59f, 0.98f, 0.67f};
    style->Colors[ImGuiCol_TitleBg]               = (ImVec4){0.96f, 0.96f, 0.96f, 1.00f};
    style->Colors[ImGuiCol_TitleBgCollapsed]      = (ImVec4){1.00f, 1.00f, 1.00f, 0.51f};
    style->Colors[ImGuiCol_TitleBgActive]         = (ImVec4){0.82f, 0.82f, 0.82f, 1.00f};
    style->Colors[ImGuiCol_MenuBarBg]             = (ImVec4){0.86f, 0.86f, 0.86f, 1.00f};
    style->Colors[ImGuiCol_ScrollbarBg]           = (ImVec4){0.98f, 0.98f, 0.98f, 0.53f};
    style->Colors[ImGuiCol_ScrollbarGrab]         = (ImVec4){0.69f, 0.69f, 0.69f, 0.80f};
    style->Colors[ImGuiCol_ScrollbarGrabHovered]  = (ImVec4){0.49f, 0.49f, 0.49f, 0.80f};
    style->Colors[ImGuiCol_ScrollbarGrabActive]   = (ImVec4){0.49f, 0.49f, 0.49f, 1.00f};
    //style->Colors[ImGuiCol_ComboBg]               = (ImVec4){0.86f, 0.86f, 0.86f, 0.99f};
    style->Colors[ImGuiCol_CheckMark]             = (ImVec4){0.26f, 0.59f, 0.98f, 1.00f};
    style->Colors[ImGuiCol_SliderGrab]            = (ImVec4){0.26f, 0.59f, 0.98f, 0.78f};
    style->Colors[ImGuiCol_SliderGrabActive]      = (ImVec4){0.26f, 0.59f, 0.98f, 1.00f};
    style->Colors[ImGuiCol_Button]                = (ImVec4){0.26f, 0.59f, 0.98f, 0.40f};
    style->Colors[ImGuiCol_ButtonHovered]         = (ImVec4){0.26f, 0.59f, 0.98f, 1.00f};
    style->Colors[ImGuiCol_ButtonActive]          = (ImVec4){0.06f, 0.53f, 0.98f, 1.00f};
    style->Colors[ImGuiCol_Header]                = (ImVec4){0.26f, 0.59f, 0.98f, 0.31f};
    style->Colors[ImGuiCol_HeaderHovered]         = (ImVec4){0.26f, 0.59f, 0.98f, 0.80f};
    style->Colors[ImGuiCol_HeaderActive]          = (ImVec4){0.26f, 0.59f, 0.98f, 1.00f};
    //style->Colors[ImGuiCol_Column]                = (ImVec4){0.39f, 0.39f, 0.39f, 1.00f};
    //style->Colors[ImGuiCol_ColumnHovered]         = (ImVec4){0.26f, 0.59f, 0.98f, 0.78f};
    //style->Colors[ImGuiCol_ColumnActive]          = (ImVec4){0.26f, 0.59f, 0.98f, 1.00f};
    style->Colors[ImGuiCol_ResizeGrip]            = (ImVec4){1.00f, 1.00f, 1.00f, 0.00f};
    style->Colors[ImGuiCol_ResizeGripHovered]     = (ImVec4){0.26f, 0.59f, 0.98f, 0.67f};
    style->Colors[ImGuiCol_ResizeGripActive]      = (ImVec4){0.26f, 0.59f, 0.98f, 0.95f};
    //style->Colors[ImGuiCol_CloseButton]           = (ImVec4){0.59f, 0.59f, 0.59f, 0.50f};
    //style->Colors[ImGuiCol_CloseButtonHovered]    = (ImVec4){0.98f, 0.39f, 0.36f, 1.00f};
    //style->Colors[ImGuiCol_CloseButtonActive]     = (ImVec4){0.98f, 0.39f, 0.36f, 1.00f};
    style->Colors[ImGuiCol_PlotLines]             = (ImVec4){0.39f, 0.39f, 0.39f, 1.00f};
    style->Colors[ImGuiCol_PlotLinesHovered]      = (ImVec4){1.00f, 0.43f, 0.35f, 1.00f};
    style->Colors[ImGuiCol_PlotHistogram]         = (ImVec4){0.90f, 0.70f, 0.00f, 1.00f};
    style->Colors[ImGuiCol_PlotHistogramHovered]  = (ImVec4){1.00f, 0.60f, 0.00f, 1.00f};
    style->Colors[ImGuiCol_TextSelectedBg]        = (ImVec4){0.26f, 0.59f, 0.98f, 0.35f};
    //style->Colors[ImGuiCol_TooltipBg]             = (ImVec4){1.00f, 1.00f, 1.00f, 0.94f};
    //style->Colors[ImGuiCol_ModalWindowDarkening]  = (ImVec4){0.20f, 0.20f, 0.20f, 0.35f};
}

static bool showMetrics = false;
static bool showAbout = false;

static void AboutWindow(bool *p_open);
static void EditorWindow(struct EdSettings *settings);

bool DoGui(struct EdSettings *settings, struct Map *map)
{
    bool doQuit = false;
    if(igBeginMainMenuBar())
    {
        if(igBeginMenu("File", true))
        {
            if(igMenuItem_Bool("New", "Ctrl+N", false, true) || igShortcut(ImGuiMod_Ctrl | ImGuiKey_N, 0, 0)) { printf("New Map!\n"); }
            if(igMenuItem_Bool("Open", "Ctrl+O", false, true) || igShortcut(ImGuiMod_Ctrl | ImGuiKey_O, 0, 0)) { printf("Open Map!\n"); }
            if(igMenuItem_Bool("Save", "Ctrl+S", false, true) || igShortcut(ImGuiMod_Ctrl | ImGuiKey_S, 0, 0)) { printf("Save Map!\n"); }
            if(igMenuItem_Bool("SaveAs", "", false, true)) { printf("Save Map As!\n"); }
            igSeparator();
            if(igMenuItem_Bool("Export", "Ctrl+E", false, true) || igShortcut(ImGuiMod_Ctrl | ImGuiKey_E, 0, 0)) {  }
            igSeparator();
            if(igMenuItem_Bool("Quit", "Alt+F4", false, true) || igShortcut(ImGuiMod_Alt | ImGuiKey_F4, 0, 0)) { doQuit = true; }
            igEndMenu();
        }

        if(igBeginMenu("Edit", true))
        {
            if(igMenuItem_Bool("Undo", "Ctrl+Z", false, true) || igShortcut(ImGuiMod_Ctrl | ImGuiKey_Z, 0, 0)) { printf("Undo!\n"); }
            if(igMenuItem_Bool("Redo", "Ctrl+Y", false, true) || igShortcut(ImGuiMod_Ctrl | ImGuiKey_Y, 0, 0)) { printf("Redo!\n"); }
            igSeparator();
            if(igMenuItem_Bool("Settings", "", false, true)) {  }
            igEndMenu();
        }

        if(igBeginMenu("Connect", false))
        {
            igEndMenu();
        }

        if(igBeginMenu("Help", true))
        {
            if(igMenuItem_Bool("Show Metrics", "", showMetrics, true)) { showMetrics = !showMetrics; }
            if(igMenuItem_Bool("About", "", showAbout, true)) { showAbout = !showAbout; }
            igEndMenu();
        }

        igEndMainMenuBar();
    }

    //EditorWindow(settings);

    if(showAbout)
        AboutWindow(&showAbout);

    if(showMetrics)
        igShowMetricsWindow(&showMetrics);

    return doQuit;
}

static void AboutWindow(bool *p_open)
{
    if(igBegin("About", p_open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
    {
        igText("Editor2\nA map editor for the WeekRPG project\nMade by Novalis");
        igEnd();
    }
}

static void EditorWindow(struct EdSettings *settings)
{
    ImGuiViewport *vp = igGetMainViewport();
    igSetNextWindowSize(vp->WorkSize, 0);
    igSetNextWindowPos(vp->WorkPos, 0, (ImVec2){ 0, 0 });

    igBegin("Editor", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);
    igButton("A", (ImVec2){ 24, 24 });
    igSameLine(0, 4);
    igButton("B", (ImVec2){ 24, 24 });
    igSameLine(0, 4);
    igButton("C", (ImVec2){ 24, 24 });
    igSameLine(0, 4);
    igButton("D", (ImVec2){ 24, 24 });

    igBeginChild_ID(1000, (ImVec2){ 0, 0 }, false, 0);
    ImVec2 clientArea;
    igGetWindowSize(&clientArea);

    ImVec2 clientPos;
    igGetWindowPos(&clientPos);

    if(igIsMouseClicked_Bool(ImGuiMouseButton_Left, false)) 
    {
        ImVec2 mpos;
        igGetMousePos(&mpos);
        int relX = (int)mpos.x - (int)clientPos.x;
        int relY = (int)mpos.y - (int)clientPos.y;
        
        if(igIsWindowFocused(0) && (relX >= 0 && relX < clientArea.x && relY >= 0 && relY < clientArea.y))
            printf("Click: %d | %d\n", relX,  relY);
    }

    ResizeEditor(settings, clientArea.x, clientArea.y);
    igImage((void*)(intptr_t)settings->editorColorTexture, clientArea, (ImVec2){ 0, 0 }, (ImVec2){ 1, 1 }, (ImVec4){ 1, 1, 1, 1 }, (ImVec4){ 1, 1, 1, 0 });
    igEndChild();

    igEnd();
}
