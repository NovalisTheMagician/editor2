#include "common.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_USE_OPENGL3
#define CIMGUI_USE_SDL2
#include "cimgui.h"
#include "cimgui_impl.h"

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include <ftplib.h>
#include <unistd.h>

#include "editor.h"
#include "map.h"
#include "gui.h"
#include "async_load.h"

#include "texture_load.h"

#include "resources/resources.h"

#define SETTINGS_FILE "./settings.ini"
#define DEFAULT_WINDOW_WIDTH 1600
#define DEFAULT_WINDOW_HEIGHT 900

static SDL_Window* InitSDL(void);
static bool InitImgui(SDL_Window *window, SDL_GLContext context);
static SDL_GLContext InitOpenGL(SDL_Window *window);

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
            string_free(state->project.file);
            state->project.file = string_cstr(optarg);
            if(!LoadProject(&state->project))
            {
                LogWarning("failed to load project %s", optarg);
                NewProject(&state->project);
            }
            break;
        case 'm':
            string_free(state->map.file);
            state->map.file = string_cstr(optarg);
            if(!LoadMap(&state->map))
            {
                LogWarning("failed to load map %s", optarg);
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

int EditorMain(int argc, char *argv[])
{
#if defined(_DEBUG)
    debug_init("memory_logs.txt");
#endif

    atexit(SDL_Quit);

    FtpInit();

    SDL_Window *window = InitSDL();
    if(!window) return EXIT_FAILURE;

    SDL_GLContext glContext = InitOpenGL(window);
    if(!glContext) return EXIT_FAILURE;

    if(!InitImgui(window, glContext)) return EXIT_FAILURE;

    InitGui();

    EdState *state = calloc(1, sizeof *state);
    LogInit(&state->log);

    LogInfo("OpenGL Version %s", glGetString(GL_VERSION));
    LogInfo("OpenGL Renderer %s", glGetString(GL_RENDERER));
    LogInfo("OpenGL Vendor %s", glGetString(GL_VENDOR));
    LogInfo("OpenGL GLSL %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

    ResetSettings(&state->settings);
    NewProject(&state->project);
    NewMap(&state->map);
    HandleArguments(argc, argv, state);

    if(!InitEditor(state)) return EXIT_FAILURE;

    tc_init(&state->textures);
    if(string_length(state->project.file) > 0)
    {
        LoadTextures(state, true);
    }

    SetStyle(state->settings.theme);
    ImGuiIO *ioptr = igGetIO();

    SDL_Event e;
    bool quit = false;
    while(!quit)
    {
        while(SDL_PollEvent(&e) > 0)
        {
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

        if(state->gl.editorColorTexture > 0)
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

        if(state->ui.show3dView && state->gl.realtimeColorTexture > 0)
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

    LogDestroy(&state->log);

    free(state);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    igDestroyContext(NULL);

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);

#if defined(_DEBUG)
    debug_finish(-1);
#endif

    return EXIT_SUCCESS;
}

static SDL_Window* InitSDL(void)
{
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        const char *errMsg = SDL_GetError();
        printf("failed to init sdl: %s\n", errMsg);
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

    uint32_t flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
#ifndef _DEBUG
    flags |= SDL_WINDOW_MAXIMIZED;
#endif

    SDL_Window *window = SDL_CreateWindow("Editor2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, flags);
    if(!window)
    {
        const char *errMsg = SDL_GetError();
        printf("failed to create window: %s\n", errMsg);
        return NULL;
    }

    SDL_EnableScreenSaver();

    return window;
}

static bool InitImgui(SDL_Window *window, SDL_GLContext context)
{
    igCreateContext(NULL);

    ImGuiIO *ioptr = igGetIO();
    ioptr->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
#if defined(USE_DOCKING)
    ioptr->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#endif

    ImFontConfig *config = ImFontConfig_ImFontConfig();
    config->FontDataOwnedByAtlas = false;
    ImFontAtlas_AddFontFromMemoryTTF(ioptr->Fonts, (void*)gFontData, gFontSize, 16.75f, config, NULL);

    ioptr->IniFilename = NULL;

    if(!ImGui_ImplSDL2_InitForOpenGL(window, context))
        return false;
    if(!ImGui_ImplOpenGL3_Init(SHADER_VERSION))
        return false;

    return true;
}

static SDL_GLContext InitOpenGL(SDL_Window *window)
{
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if(!glContext)
    {
        const char *errMsg = SDL_GetError();
        printf("failed to create context: %s\n", errMsg);
        return NULL;
    }

    if(SDL_GL_MakeCurrent(window, glContext) != 0)
    {
        const char *errMsg = SDL_GetError();
        printf("failed to make context current: %s\n", errMsg);
        return NULL;
    }

    if(!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress))
    {
        printf("couldn't load GL functions\n");
        return NULL;
    }

    if(SDL_GL_SetSwapInterval(-1) == -1)
        SDL_GL_SetSwapInterval(1);

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);

    return glContext;
}
