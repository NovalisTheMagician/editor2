#include "gui.h"

#include "edit.h"

#include <tgmath.h>
#include <assert.h>
#include <ImGuiFileDialog.h>

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

static void AboutWindow(bool *p_open);
static void SettingsWindow(bool *p_open, struct EdState *state);
static void ToolbarWindow(bool *p_open, struct EdState *state);
static void EditorWindow(bool *p_open, struct EdState *state);
static void RealtimeWindow(bool *p_open, struct EdState *state);
static void StatsWindow(bool *p_open, struct EdState *state);
static void MainMenuBar(bool *doQuit, struct EdState *state);
static void ProjectSettingsWindow(bool *p_open, struct EdState *state);
static void MapSettingsWindow(bool *p_open, struct EdState *state);
static void FileDialog(bool *doQuit);

static void ProjectSavePopup(struct EdState *state, bool *quitRequest);
static void MapSavePopup(struct EdState *state, bool *quitRequest);

static void HandleShortcuts(struct EdState *state);

enum SaveModalAction
{
    SMA_NEW,
    SMA_OPEN,
    SMA_QUIT
};

struct FileDialogAction 
{
    void *data;
    void (*callback)(const char *path, void *data);
    bool quitRequest;
};

static ImGuiFileDialog *cfileDialog;

static void OpenFolderCallback(const char *path, void *data)
{
    pstring *str = data;
    size_t len = strlen(path);
    memset(str->data, 0, str->size);
    memcpy(str->data, path, len < str->size ? len : str->size);
}

static void OpenFolderDialog(pstring *folderPath)
{
    struct FileDialogAction *fda = calloc(1, sizeof *fda);
    fda->data = folderPath;
    fda->callback = OpenFolderCallback;
    IGFD_OpenDialog(cfileDialog, "filedlg", "Choose a Folder", NULL, ".", "", 1, fda, ImGuiFileDialogFlags_Modal);
}

static void SaveMapCallback(const char *path, void *data)
{
    struct Map *map = data;
    pstr_free(map->file);
    map->file = pstr_cstr(path);
    SaveMap(map);
}

static void SaveMapDialog(struct Map *map, bool quitRequest)
{
    struct FileDialogAction *fda = calloc(1, sizeof *fda);
    fda->data = map;
    fda->callback = SaveMapCallback;
    fda->quitRequest = quitRequest;
    IGFD_OpenDialog(cfileDialog, "filedlg", "Save Map", "Map Files(*.map){.map}", ".", "", 1, fda, ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_ConfirmOverwrite | ImGuiFileDialogFlags_CaseInsensitiveExtention);
}

static void SaveProjectCallback(const char *path, void *data)
{
    struct Project *project = data;
    pstr_free(project->file);
    project->file = pstr_cstr(path);
    SaveProject(project);
}

static void SaveProjectDialog(struct Project *project, bool quitRequest)
{
    struct FileDialogAction *fda = calloc(1, sizeof *fda);
    fda->data = project;
    fda->callback = SaveProjectCallback;
    fda->quitRequest = true;
    IGFD_OpenDialog(cfileDialog, "filedlg", "Save Project", "Project Files(*.pro){.pro}", ".", "", 1, fda, ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_ConfirmOverwrite | ImGuiFileDialogFlags_CaseInsensitiveExtention);
}

static void OpenMapCallback(const char *path, void *data)
{
    struct Map *map = data;
    pstr_free(map->file);
    map->file = pstr_cstr(path);
    LoadMap(map);
}

static void OpenMapDialog(struct Map *map)
{
    struct FileDialogAction *fda = calloc(1, sizeof *fda);
    fda->data = map;
    fda->callback = OpenMapCallback;
    IGFD_OpenDialog(cfileDialog, "filedlg", "Open Map", "Map Files(*.map){.map}, All(*.*){.*}", ".", "", 1, fda, ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_ReadOnlyFileNameField | ImGuiFileDialogFlags_CaseInsensitiveExtention);
}

static void OpenProjectCallback(const char *path, void *data)
{
    struct Project *project = data;
    pstr_free(project->file);
    project->file = pstr_cstr(path);
    LoadProject(project);
}

static void OpenProjectDialog(struct Project *project)
{
    struct FileDialogAction *fda = calloc(1, sizeof *fda);
    fda->data = project;
    fda->callback = OpenProjectCallback;
    IGFD_OpenDialog(cfileDialog, "filedlg", "Open Map", "Project Files(*.pro){.pro}, All(*.*){.*}", ".", "", 1, fda, ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_ReadOnlyFileNameField | ImGuiFileDialogFlags_CaseInsensitiveExtention);
}

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

bool DoGui(struct EdState *state, bool doQuit)
{
    openProjectPopup = false;
    openMapPopup = false;

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

    if(state->ui.showStats)
        StatsWindow(&state->ui.showStats, state);

    if(state->ui.showProjectSettings)
        ProjectSettingsWindow(&state->ui.showProjectSettings, state);

    if(state->ui.showMapSettings)
        MapSettingsWindow(&state->ui.showMapSettings, state);

    EditorWindow(NULL, state);

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

    return doQuit;
}

static void MainMenuBar(bool *doQuit, struct EdState *state)
{
    bool allowFileOps = state->network.hosting || !state->network.connected;
    if(igBeginMainMenuBar())
    {
        float menubarHeight = igGetWindowHeight();
        if(igBeginMenu("File", true))
        {
            if(igMenuItem_Bool("New Project", "", false, allowFileOps)) { if(state->project.dirty) { openProjectPopup = true; modalAction = SMA_NEW; } else NewProject(&state->project); }
            if(igMenuItem_Bool("Open Project", "", false, allowFileOps)) { if(state->project.dirty) { openProjectPopup = true; modalAction = SMA_OPEN; } else OpenProjectDialog(&state->project); }
            if(igMenuItem_Bool("Save Project", "", false, state->project.dirty && allowFileOps)) 
            { 
                if(state->project.file.size == 0)
                    SaveProjectDialog(&state->project, false);
                else
                    SaveProject(&state->project);
            }
            igSeparator();
            if(igMenuItem_Bool("New Map", "Ctrl+N", false, allowFileOps)) { if(state->map.dirty) { openMapPopup = true; modalAction = SMA_NEW; } else NewMap(&state->map); }
            if(igMenuItem_Bool("Open Map", "Ctrl+O", false, allowFileOps)) { if(state->map.dirty) { openMapPopup = true; modalAction = SMA_OPEN; } else OpenMapDialog(&state->map); }
            if(igMenuItem_Bool("Save Map", "Ctrl+S", false, state->map.dirty && allowFileOps)) 
            { 
                if(state->map.file.size == 0)
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
            if(igMenuItem_Bool("Undo", "Ctrl+Z", false, true)) { printf("Undo!\n"); }
            if(igMenuItem_Bool("Redo", "Ctrl+Y", false, true)) { printf("Redo!\n"); }
            igSeparator();
            if(igMenuItem_Bool("Copy", "Ctrl+C", false, true)) { EditCopy(state); }
            if(igMenuItem_Bool("Paste", "Ctrl+V", false, true)) { EditPaste(state); }
            if(igMenuItem_Bool("Cut", "Ctrl+X", false, true)) { EditCut(state); }
            igSeparator();
            if(igBeginMenu("Modes", true))
            {
                if(igMenuItem_Bool("Vertex", "V", state->data.selectionMode == MODE_VERTEX, true)) { state->data.selectionMode = MODE_VERTEX; }
                if(igMenuItem_Bool("Line", "L", state->data.selectionMode == MODE_LINE, true)) { state->data.selectionMode = MODE_LINE; }
                if(igMenuItem_Bool("Sector", "S", state->data.selectionMode == MODE_SECTOR, true)) { state->data.selectionMode = MODE_SECTOR; }
                igEndMenu();
            }
            igSeparator();
            igMenuItem_BoolPtr("Map Settings", "", &state->ui.showMapSettings, true);
            igMenuItem_BoolPtr("Project Settings", "", &state->ui.showProjectSettings, true);
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
            igMenuItem_BoolPtr("Toolbar", "", &state->ui.showToolbar, true);
            igMenuItem_BoolPtr("Textures", "Ctrl+T", &state->ui.showTextures, true);
            igMenuItem_BoolPtr("Entities", "Ctrl+E", &state->ui.showEntities, true);
            igMenuItem_BoolPtr("3D View", "Ctrl+W", &state->ui.show3dView, true);
            igSeparator();
            igMenuItem_BoolPtr("Logs", "Ctrl+L", &state->ui.showLogs, true);
            igSeparator();
            if(igBeginMenu("Arrange", true))
            {
                ImVec2 displaySize = igGetIO()->DisplaySize;
                if(igMenuItem_Bool("Fullsize", "", false, true)) 
                { 
                    ImVec2 size = { .x = displaySize.x, .y = displaySize.y - menubarHeight };
                    ImVec2 pos = { .x = 0, .y = menubarHeight };
                    igSetWindowSize_Str("Editor", size, 0);
                    igSetWindowPos_Str("Editor", pos, 0);
                }
                if(igMenuItem_Bool("Side by Side", "", false, true)) 
                { 
                    ImVec2 edsize = { .x = displaySize.x * 0.60f, .y = displaySize.y - menubarHeight };
                    ImVec2 edpos = { .x = 0, .y = menubarHeight };
                    igSetWindowSize_Str("Editor", edsize, 0);
                    igSetWindowPos_Str("Editor", edpos, 0);
                    ImVec2 rtsize = { .x = displaySize.x * 0.40f, .y = displaySize.y - menubarHeight };
                    ImVec2 rtpos = { .x = displaySize.x * 0.60f, .y = menubarHeight };
                    igSetWindowSize_Str("3D View", rtsize, 0);
                    igSetWindowPos_Str("3D View", rtpos, 0);
                }
                if(igMenuItem_Bool("Texturing", "", false, true)) 
                { 
                    ImVec2 rtsize = { .x = displaySize.x * 0.65f, .y = displaySize.y - menubarHeight };
                    ImVec2 rtpos = { .x = 0, .y = menubarHeight };
                    igSetWindowSize_Str("3D View", rtsize, 0);
                    igSetWindowPos_Str("3D View", rtpos, 0);
                }
                igEndMenu();
            }
#ifdef _DEBUG
            igSeparator();
            igMenuItem_BoolPtr("Stats", "", &state->ui.showStats, true);
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
    igSetNextWindowSize((ImVec2){ 600, 300 }, ImGuiCond_FirstUseEver);
    if(igBegin("Options", p_open, 0))
    {
        if(igBeginTabBar("", 0))
        {
            if(igBeginTabItem("General", NULL, 0))
            {
                igSeparatorText("Game");
                igInputText("Gamepath", state->settings.gamePath.data, state->settings.gamePath.size, 0, NULL, NULL);
                igInputText("Launch Arguments", state->settings.launchArguments.data, state->settings.launchArguments.size, 0, NULL, NULL);
                igSeparatorText("Other");
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

static void ToolbarWindow(bool *p_open, struct EdState *state)
{
    if(igBegin("Toolbar", p_open, 0))
    {
        
    }
    igEnd();
}

static void EditorWindow(bool *p_open, struct EdState *state)
{
    static bool firstTime = true;

    igSetNextWindowSize((ImVec2){ 800, 600 }, ImGuiCond_FirstUseEver);
    igSetNextWindowPos((ImVec2){ 40, 40 }, ImGuiCond_FirstUseEver, (ImVec2){ 0, 0 });

    igPushStyleVar_Vec2(ImGuiStyleVar_WindowMinSize, (ImVec2){ 400, 300 });
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar;
    if(state->map.dirty)
        flags |= ImGuiWindowFlags_UnsavedDocument;

    if(igBegin("Editor", p_open, flags))
    {
        if(igShortcut(ImGuiMod_Ctrl | ImGuiKey_C, 0, 0))
            EditCopy(state);
        if(igShortcut(ImGuiMod_Ctrl | ImGuiKey_V, 0, 0))
            EditPaste(state);
        if(igShortcut(ImGuiMod_Ctrl | ImGuiKey_X, 0, 0))
            EditCut(state);
        if(igShortcut(ImGuiMod_Ctrl | ImGuiKey_Z, 0, 0))
            printf("Undo!\n");
        if(igShortcut(ImGuiMod_Ctrl | ImGuiKey_Y, 0, 0))
            printf("Redo!\n");

        if(igShortcut(ImGuiKey_V, 0, 0))
            state->data.selectionMode = MODE_VERTEX;
        if(igShortcut(ImGuiKey_L, 0, 0))
            state->data.selectionMode = MODE_LINE;
        if(igShortcut(ImGuiKey_S, 0, 0))
            state->data.selectionMode = MODE_SECTOR;

        igPushItemWidth(80);
        static const char *modeNames[] = { "Vertex", "Line", "Sector" };
        static const size_t numModes = COUNT_OF(modeNames);
        igCombo_Str_arr("Mode", &state->data.selectionMode, modeNames, numModes, 3);

        igSameLine(0, 16);
        igPushItemWidth(80);
        static const char *gridSizes[] = { "1", "2", "4", "8", "16", "32", "64", "128", "256", "512", "1024" };
        static const size_t numGrids = COUNT_OF(gridSizes);
        int gridSelection = log2(state->data.gridSize);
        igCombo_Str_arr("Gridsize", &gridSelection, gridSizes, numGrids, numGrids);
        state->data.gridSize = pow(2, gridSelection);

        igSameLine(0, 16);
        igPushItemWidth(80);
        igSliderFloat("Zoom", &state->data.zoomLevel, MIN_ZOOM, MAX_ZOOM, "%.2f", 0);

        igSameLine(0, 16);
        if(igButton("Reset Zoom", (ImVec2){ 0, 0 })) { state->data.zoomLevel = 1; }

        igSameLine(0, 16);
        if(igButton("Go To Origin", (ImVec2){ 0, 0 })) 
        {
            state->data.viewPosition = (ImVec2){ -state->gl.editorFramebufferWidth / 2, -state->gl.editorFramebufferHeight / 2 };
        }

        if(igBeginChild_ID(1000, (ImVec2){ 0, 0 }, false, ImGuiWindowFlags_NoMove))
        {
            ImVec2 clientArea;
            igGetContentRegionAvail(&clientArea);

            ImVec2 clientPos;
            igGetWindowPos(&clientPos);

            bool hovored = igIsWindowHovered(0);
            bool focused = igIsWindowFocused(0);

            ImVec2 mpos;
            igGetMousePos(&mpos);
            int relX = (int)mpos.x - (int)clientPos.x;
            int relY = (int)mpos.y - (int)clientPos.y;

            if(hovored)
            {
                int edX = relX, edSX = relX, edY = relY, edSY = relY;
                float edXf = relX, edYf = relY;
                ScreenToEditorSpaceGrid(state, &edSX, &edSY);
                ScreenToEditorSpace(state, &edX, &edY);
                ScreenToEditorSpacef(state, &edXf, &edYf);
#ifdef _DEBUG
                state->data.mx = edX;
                state->data.my = edY;
                state->data.mtx = edSX;
                state->data.mty = edSY;
#endif

                if(igIsMouseDragging(ImGuiMouseButton_Middle, 2))
                {
                    ImVec2 dragDelta;
                    igGetMouseDragDelta(&dragDelta, ImGuiMouseButton_Middle, 2);
                    state->data.viewPosition.x -= dragDelta.x;
                    state->data.viewPosition.y -= dragDelta.y;
                    igResetMouseDragDelta(ImGuiMouseButton_Middle);

                    igSetWindowFocus_Nil();
                }

                if(igIsMouseClicked_Bool(ImGuiMouseButton_Left, false)) 
                {
                    
                }

                if(igIsMouseClicked_Bool(ImGuiMouseButton_Right, false)) 
                {
                    size_t newVertex;
                    if(!EditGetVertex(state, (struct Vertex){ edSX, edSY }, &newVertex))
                    {
                        newVertex = EditAddVertex(state, (struct Vertex){ edSX, edSY });
                    }
                    if(state->data.lastVertForLine != -1)
                    {
                        EditAddLine(state, state->data.lastVertForLine, newVertex);
                    }

                    state->data.lastVertForLine = newVertex;

                    igSetWindowFocus_Nil();
                }

                if(igIsMouseClicked_Bool(ImGuiMouseButton_Middle, false)) 
                {
                    igSetWindowFocus_Nil();
                }

                ImGuiKeyData *wheelData = igGetKeyData_Key(ImGuiKey_MouseWheelY);
                if(wheelData->AnalogValue != 0)
                {
                    state->data.zoomLevel += wheelData->AnalogValue * 0.05f;
                    if(state->data.zoomLevel < MIN_ZOOM)
                        state->data.zoomLevel = MIN_ZOOM;
                    if(state->data.zoomLevel > MAX_ZOOM)
                        state->data.zoomLevel = MAX_ZOOM;

                    float edXAfter = relX, edYAfter = relY;
                    ScreenToEditorSpacef(state, &edXAfter, &edYAfter);

                    state->data.viewPosition.x += (edXf - edXAfter) * state->data.zoomLevel;
                    state->data.viewPosition.y += (edYf - edYAfter) * state->data.zoomLevel;

                    igSetWindowFocus_Nil();
                }
            }

            if(focused)
            {
                
            }

            ResizeEditorView(state, clientArea.x, clientArea.y);
            igImage((void*)(intptr_t)state->gl.editorColorTexture, clientArea, (ImVec2){ 0, 0 }, (ImVec2){ 1, 1 }, (ImVec4){ 1, 1, 1, 1 }, (ImVec4){ 1, 1, 1, 0 });

            if(firstTime)
            {
                firstTime = false;
                state->data.viewPosition = (ImVec2){ -state->gl.editorFramebufferWidth / 2, -state->gl.editorFramebufferHeight / 2 };
            }
        }
        igEndChild();
    }
    igEnd();
    igPopStyleVar(1);
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

            ImVec2 mpos;
            igGetMousePos(&mpos);
            int relX = (int)mpos.x - (int)clientPos.x;
            int relY = (int)mpos.y - (int)clientPos.y;
            (void)relX;
            (void)relY;

            if(hovored)
            {

            }

            if(focused)
            {

            }

            ResizeRealtimeView(state, clientArea.x, clientArea.y);
            igImage((void*)(intptr_t)state->gl.realtimeColorTexture, clientArea, (ImVec2){ 0, 0 }, (ImVec2){ 1, 1 }, (ImVec4){ 1, 1, 1, 1 }, (ImVec4){ 1, 1, 1, 0 });
        }
        igEndChild();
    }
    igEnd();
}

static void StatsWindow(bool *p_open, struct EdState *state)
{
    if(igBegin("Debug Stats", p_open, 0))
    {
        igSeparatorText("Editor");
        igText("Viewposition: %.2f | %.2f", state->data.viewPosition.x, state->data.viewPosition.y);
#ifdef _DEBUG
        igText("Mouse World Pos Snap: %d | %d", state->data.mtx, state->data.mty);
        igText("Mouse World Pos: %d | %d", state->data.mx, state->data.my);
#endif
        igText("Zoomlevel: %.2f", state->data.zoomLevel);
    }
    igEnd();
}

static void ProjectSavePopup(struct EdState *state, bool *quitRequest)
{
    if(igBeginPopupModal("Save Project?", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
    {
        igText("You have unsaved changes to the Project.\nDo you want to save them?");
        if(igButton("Yes", (ImVec2){ 64, 0 }))
        {
            if(state->project.file.size > 0)
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

static void MapSavePopup(struct EdState *state, bool *quitRequest)
{
    if(igBeginPopupModal("Save Map?", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
    {
        igText("You have unsaved changes to the Map.\nDo you want to save them?");
        if(igButton("Yes", (ImVec2){ 64, 0 }))
        {
            if(state->map.file.size > 0)
                SaveMap(&state->map);

            switch(modalAction)
            {
            case SMA_NEW: NewMap(&state->map); break;
            case SMA_OPEN: OpenMapDialog(&state->map); break;
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
            case SMA_NEW: NewMap(&state->map); break;
            case SMA_OPEN: OpenMapDialog(&state->map); break;
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

static void ProjectSettingsWindow(bool *p_open, struct EdState *state)
{
    igSetNextWindowSize((ImVec2){ 400, 290 }, ImGuiCond_FirstUseEver);
    if(igBegin("Project Settings", p_open, 0))
    {

        igSeparatorText("Base");
        bool isFtp = state->project.basePath.type == ASSPATH_FTP;
        igCheckbox("FTP", &isFtp);
        state->project.basePath.type = isFtp ? ASSPATH_FTP : ASSPATH_FS;
        if(isFtp)
        {
            igSameLine(0, 8);
            if(igButton("Check Connection", (ImVec2){ 0, 0 }))
            {

            }
        }
        if(igInputText("Path", state->project.basePath.fs.path.data, state->project.basePath.fs.path.size, 0, NULL, NULL)) { state->project.dirty = true; }
        igSameLine(0, 8);
        if(igButton("Browse", (ImVec2){ 0, 0 }))
        {
            if(!isFtp)
            {
                OpenFolderDialog(&state->project.basePath.fs.path);
                state->project.dirty = true;
            }
        }
        if(isFtp)
        {
            if(igInputText("URL", state->project.basePath.ftp.url.data, state->project.basePath.ftp.url.size, 0, NULL, NULL)) { state->project.dirty = true; }
            if(igInputText("Login", state->project.basePath.ftp.login.data, state->project.basePath.ftp.login.size, 0, NULL, NULL)) { state->project.dirty = true; }
            if(igInputText("Password", state->project.basePath.ftp.password.data, state->project.basePath.ftp.password.size, 0, NULL, NULL)) { state->project.dirty = true; }
        }

        igSeparatorText("Textures");
        igPushID_Str("Textures");
        if(igInputText("Subpath", state->project.texturesPath.data, state->project.texturesPath.size, 0, NULL, NULL)) { state->project.dirty = true; }
        igPopID();

        igSeparatorText("Things");
        igPushID_Str("Things");
        if(igInputText("Subpath", state->project.thingsFile.data, state->project.thingsFile.size, 0, NULL, NULL)) { state->project.dirty = true; }
        igPopID();
    }
    igEnd();
}

static void MapSettingsWindow(bool *p_open, struct EdState *state)
{
    if(igBegin("Map Settings", p_open, 0))
    {
        igSliderInt("Texture Scale", &state->map.textureScale, 1, 10, NULL, 0);
    }
    igEnd();
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
        struct FileDialogAction *action = IGFD_GetUserDatas(cfileDialog);
        assert(action);
        if(IGFD_IsOk(cfileDialog))
        {
            char* cfilePathName = IGFD_GetFilePathName(cfileDialog, IGFD_ResultMode_AddIfNoFileExt);
            action->callback(cfilePathName, action->data);
            if (cfilePathName) free(cfilePathName);
            *doQuit = action->quitRequest;
        }
        
        free(action);
        IGFD_CloseDialog(cfileDialog);
    }
}

static void HandleShortcuts(struct EdState *state)
{
    if(igShortcut(ImGuiMod_Ctrl | ImGuiKey_N, 0, ImGuiInputFlags_RouteGlobalLow))
    {
        if(state->map.dirty) { openMapPopup = true; modalAction = SMA_NEW; }
        else NewMap(&state->map);
    }

    if(igShortcut(ImGuiMod_Ctrl | ImGuiKey_O, 0, ImGuiInputFlags_RouteGlobalLow))
    {
        if(state->map.dirty) { openMapPopup = true; modalAction = SMA_OPEN; }
        else OpenMapDialog(&state->map);
    }

    if(igShortcut(ImGuiMod_Ctrl | ImGuiKey_S, 0, ImGuiInputFlags_RouteGlobalLow))
    {
        if(state->map.dirty)
        {
            if(state->map.file.size == 0)
                SaveMapDialog(&state->map, false);
            else
                SaveMap(&state->map);
        }
    }

    if(igShortcut(ImGuiMod_Ctrl | ImGuiKey_W, 0, ImGuiInputFlags_RouteGlobalLow))
    {
        state->ui.show3dView = !state->ui.show3dView;
    }

    if(igShortcut(ImGuiMod_Ctrl | ImGuiKey_E, 0, ImGuiInputFlags_RouteGlobalLow))
    {
        state->ui.showEntities = !state->ui.showEntities;
    }

    if(igShortcut(ImGuiMod_Ctrl | ImGuiKey_T, 0, ImGuiInputFlags_RouteGlobalLow))
    {
        state->ui.showTextures = !state->ui.showTextures;
    }

    if(igShortcut(ImGuiMod_Ctrl | ImGuiKey_L, 0, ImGuiInputFlags_RouteGlobalLow))
    {
        state->ui.showLogs = !state->ui.showLogs;
    }
}
