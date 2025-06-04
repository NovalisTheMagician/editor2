#include <string.h>
#include <time.h>
#include <unistd.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_USE_OPENGL3
#define CIMGUI_USE_SDL2
#include "cimgui.h"
#include "cimgui_impl.h"

#define ARENA_IMPLEMENTATION
#include "arena.h"

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_messagebox.h>

#include "glad/gl.h"

#include <ftplib.h>

#include <stb/stb_image.h>

#include "editor.h"
#include "map.h"
#include "gui.h"
#include "async_load.h"
#include "texture_load.h"
#include "resources/resources.h"
#include "logging.h"
#include "script.h"
#include "utils/string.h"

#define SETTINGS_FILE "./settings.ini"
#define DEFAULT_WINDOW_WIDTH 1600
#define DEFAULT_WINDOW_HEIGHT 900

#define SHADER_VERSION "#version 460 core\n"
#define REQ_GL_MAJOR 4
#define REQ_GL_MINOR 6

#ifdef __APPLE__
#define DEFAULT_DPI 72.0f
#else
#define DEFAULT_DPI 96.0f
#endif

static SDL_Window* InitSDL(char *error, size_t errorSize);
static bool InitImgui(SDL_Window *window, SDL_GLContext context, char *error, size_t errorSize);
static SDL_GLContext InitOpenGL(SDL_Window *window, char *error, size_t errorSize);
static void InitFont(SDL_Window *window, bool rebuild);

static void HandleArguments(int argc, char *argv[], EdState *state)
{
    int c;
    const char *settingsPath = SETTINGS_FILE;
    while((c = getopt(argc, argv, "s:p:m:")) != -1)
    {
        switch(c)
        {
        case 's':
            settingsPath = optarg;
            break;
        case 'p':
            state->project.file = CopyString(optarg);
            if(!LoadProject(&state->project))
            {
                NewProject(&state->project);
            }
            break;
        case 'm':
            state->map.file = CopyString(optarg);
            if(!LoadMap(&state->map))
            {
                NewMap(&state->map);
            }
            break;
        }
    }

    if(!LoadSettings(settingsPath, &state->settings))
    {
        LogWarning("failed to load settings from %s! using default values", settingsPath);
    }
}

static void InitState(EdState *state)
{
    state->ui.showTextures = true;
    state->ui.showEntities = true;
    state->ui.showLogs = true;
    state->ui.showProperties = true;
}

static void setWindowIcon(SDL_Window *window)
{
    int w, h, c;
    uint8_t *pixels = stbi_load_from_memory(gIconData, gIconSize, &w, &h, &c, 4);
    SDL_Surface *surface = SDL_CreateRGBSurface(0, w, h, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
    memcpy(surface->pixels, pixels, w * h * 4);
    SDL_SetWindowIcon(window, surface);
    SDL_FreeSurface(surface);
    free(pixels);
}

static int getDisplayDpi(SDL_Window *window)
{
    float dpi;
    SDL_GetDisplayDPI(SDL_GetWindowDisplayIndex(window), &dpi, NULL, NULL);
    return (int)(dpi * 100.0f / DEFAULT_DPI);
}

int EditorMain(int argc, char *argv[])
{
    atexit(SDL_Quit);

    FtpInit();

    char errorBuffer[256] = { 0 };
    SDL_Window *window = InitSDL(errorBuffer, sizeof errorBuffer);
    if(!window)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Failed to create window", errorBuffer, NULL);
        return EXIT_FAILURE;
    }
    setWindowIcon(window);

    SDL_GLContext glContext = InitOpenGL(window, errorBuffer, sizeof errorBuffer);
    if(!glContext)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Failed to create OpenGL context", errorBuffer, window);
        SDL_DestroyWindow(window);
        return EXIT_FAILURE;
    }

    if(!InitImgui(window, glContext, errorBuffer, sizeof errorBuffer))
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Failed to initialize ImGui", errorBuffer, window);
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        return EXIT_FAILURE;
    }

    InitGui();

    EdState *state = calloc(1, sizeof *state);
    InitState(state);
    LogInit(&state->log);

    state->data.cachedDPI = getDisplayDpi(window);
    LogDebug("Display Scale: %d", state->data.cachedDPI);

    if(!InitEditor(state, errorBuffer, sizeof errorBuffer))
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Failed to init Editor", errorBuffer, window);
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        return EXIT_FAILURE;
    }

    LogInfo("OpenGL Version %s", glGetString(GL_VERSION));
    LogInfo("OpenGL Renderer %s", glGetString(GL_RENDERER));
    LogInfo("OpenGL Vendor %s", glGetString(GL_VENDOR));
    LogInfo("OpenGL GLSL %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

    ResetSettings(&state->settings);
    NewProject(&state->project);
    NewMap(&state->map);
    HandleArguments(argc, argv, state);

    if(!ScriptInit(&state->script, state))
    {
        LogWarning("Failed to initilize scripting system. Scripting won\'t be available");
    }

    tc_init(&state->textures);
    if(state->project.file)
    {
        LoadTextures(state, true);
    }

    SetStyle(state->settings.theme);
    ImGuiIO *ioptr = igGetIO_Nil();

    SDL_Event e;
    bool quit = false;
    while(!quit)
    {
        while(SDL_PollEvent(&e) > 0)
        {
            if(e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                int newDpi = getDisplayDpi(window);
                LogDebug("New Dpi: %d", newDpi);
                if(newDpi != state->data.cachedDPI)
                {
                    state->data.cachedDPI = newDpi;
                    InitFont(window, true);
                    LogDebug("Display Scale changed: %d", state->data.cachedDPI);
                }
            }

            if(ImGui_ImplSDL2_ProcessEvent(&e)) continue;

            switch(e.type)
            {
            case SDL_QUIT:
                quit = true;
                break;
            }
        }

        Async_UpdateJob(&state->async);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        igNewFrame();

        quit = DoGui(state, quit);

        if(!state->ui.render3d && state->gl.editorColorTexture > 0)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBindFramebuffer(GL_FRAMEBUFFER, state->gl.editorFramebufferMS);
            glViewport(0, 0, state->gl.editorFramebufferWidth, state->gl.editorFramebufferHeight);
            glClearNamedFramebufferfv(state->gl.editorFramebufferMS, GL_COLOR, 0, state->settings.colors[COL_BACKGROUND].raw);
            RenderEditorView(state);
            glBlitNamedFramebuffer(state->gl.editorFramebufferMS, state->gl.editorFramebuffer, 0, 0, state->gl.editorFramebufferWidth, state->gl.editorFramebufferHeight, 0, 0, state->gl.editorFramebufferWidth, state->gl.editorFramebufferHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
            glDisable(GL_BLEND);
        }

        if(state->ui.render3d && state->gl.realtimeColorTexture > 0)
        {
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);

            glBindFramebuffer(GL_FRAMEBUFFER, state->gl.realtimeFramebuffer);
            glViewport(0, 0, state->gl.realtimeFramebufferWidth, state->gl.realtimeFramebufferHeight);
            glClearNamedFramebufferfv(state->gl.realtimeFramebuffer, GL_COLOR, 0, state->settings.colors[COL_RTBACKGROUND].raw);
            const float depth = 1.0f;
            glClearNamedFramebufferfv(state->gl.realtimeFramebuffer, GL_DEPTH, 0, &depth);
            RenderRealtimeView(state);

            glDisable(GL_CULL_FACE);
            glDisable(GL_DEPTH_TEST);
        }

        igRender();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, (int)ioptr->DisplaySize.x, (int)ioptr->DisplaySize.y);
        glClearNamedFramebufferfv(0, GL_COLOR, 0, state->settings.colors[COL_WORKSPACE_BACK].raw);
        ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());

        SDL_GL_SwapWindow(window);
    }

    SaveSettings(SETTINGS_FILE, &state->settings);

    FreeGui();

    CancelFetch();
    Async_AbortJobAndWait(&state->async);

    tc_unload_all(&state->textures);
    tc_destroy(&state->textures);

    FreeMap(&state->map);
    FreeProject(&state->project);
    FreeSettings(&state->settings);
    DestroyEditor(state);

    ScriptDestroy(&state->script);

    LogDestroy(&state->log);

    free(state);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    igDestroyContext(NULL);

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}

static SDL_Window* InitSDL(char *error, size_t errorSize)
{
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        const char *errMsg = SDL_GetError();
        snprintf(error, errorSize, "%s", errMsg);
        return NULL;
    }
#ifdef _DEBUG
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, REQ_GL_MAJOR);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, REQ_GL_MINOR);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

    uint32_t flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
#ifndef _DEBUG
    flags |= SDL_WINDOW_MAXIMIZED;
#endif

    SDL_Window *window = SDL_CreateWindow("Editor2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, flags);
    if(!window)
    {
        const char *errMsg = SDL_GetError();
        snprintf(error, errorSize, "%s", errMsg);
        return NULL;
    }

    SDL_EnableScreenSaver();

    return window;
}

static void InitFont(SDL_Window *window, bool rebuild)
{
    ImGuiIO *ioptr = igGetIO_Nil();

    int displayIndex = SDL_GetWindowDisplayIndex(window);
    float dpi;
    SDL_GetDisplayDPI(displayIndex, &dpi, NULL, NULL);
    float scale = dpi / DEFAULT_DPI;
    float fontSize = floorf(16.75f * scale);

    ImFontConfig *config = ImFontConfig_ImFontConfig();
    config->FontDataOwnedByAtlas = false;
    ImFontAtlas_Clear(ioptr->Fonts);
    ImFontAtlas_AddFontFromMemoryTTF(ioptr->Fonts, (void*)gFontData, gFontSize, fontSize, config, NULL);
    ImFontAtlas_Build(ioptr->Fonts);
    ImFontConfig_destroy(config);
    //ImGuiStyle_ScaleAllSizes(igGetStyle(), scale);
    if(rebuild)
        ImGui_ImplOpenGL3_CreateFontsTexture();
}

static bool InitImgui(SDL_Window *window, SDL_GLContext context, char *error, size_t errorSize)
{
    igCreateContext(NULL);

    ImGuiIO *ioptr = igGetIO_Nil();
    ioptr->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable;

    InitFont(window, false);

    ioptr->IniFilename = NULL;

    if(!ImGui_ImplSDL2_InitForOpenGL(window, context))
    {
        snprintf(error, errorSize, "ImGui: SDL2 init failed");
        return false;
    }
    if(!ImGui_ImplOpenGL3_Init(SHADER_VERSION))
    {
        snprintf(error, errorSize, "ImGui: OpenGL3 init failed");
        return false;
    }

    return true;
}

static SDL_GLContext InitOpenGL(SDL_Window *window, char *error, size_t errorSize)
{
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if(!glContext)
    {
        const char *errMsg = SDL_GetError();
        snprintf(error, errorSize, "failed to create context: %s", errMsg);
        return NULL;
    }

    if(SDL_GL_MakeCurrent(window, glContext) != 0)
    {
        const char *errMsg = SDL_GetError();
        snprintf(error, errorSize, "failed to make context current: %s", errMsg);
        return NULL;
    }

    if(!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress))
    {
        snprintf(error, errorSize, "couldn't load GL functions");
        SDL_GL_DeleteContext(glContext);
        return NULL;
    }

    if(SDL_GL_SetSwapInterval(-1) == -1)
        SDL_GL_SetSwapInterval(1);

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);

    return glContext;
}
