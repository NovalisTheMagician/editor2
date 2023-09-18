#include "common.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_USE_OPENGL3
#define CIMGUI_USE_SDL2
#include <cimgui.h>
#include <cimgui_impl.h>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include <ftplib.h>
#include <unistd.h>

#include "editor.h"
#include "map.h"
#include "gui.h"
#include "async_load.h"

#include "texture_load.h"

#include "resources.h"

#define SETTINGS_FILE "./settings.ini"
#define DEFAULT_WINDOW_WIDTH 1600
#define DEFAULT_WINDOW_HEIGHT 900

static SDL_Window* InitSDL(void);
static bool InitImgui(SDL_Window *window, SDL_GLContext context);
static SDL_GLContext InitOpenGL(SDL_Window *window);

static void HandleArguments(int argc, char *argv[], struct EdState *state)
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
            state->project.file = pstr_cstr(optarg);
            if(!LoadProject(&state->project))
            {
                LogFormat(&state->log, LOG_WARN, "failed to load project {c}", optarg);
                NewProject(&state->project);
            }
            break;
        case 'm':
            state->map.file = pstr_cstr(optarg);
            if(!LoadMap(&state->map))
            {
                LogFormat(&state->log, LOG_WARN, "failed to load map {c}", optarg);
                NewMap(&state->map);
            }
            break;
        }
    }

    if(!LoadSettings(settingsPath, &state->settings))
    {
        LogFormat(&state->log, LOG_WARN, "failed to load settings from {c}! using default values", settingsPath);
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

    struct EdState *state = calloc(1, sizeof *state);
    LogInit(&state->log);

#if defined(_DEBUG)
    LogFormat(&state->log, LOG_INFO, "OpenGL Version {c}", glGetString(GL_VERSION));
    LogFormat(&state->log, LOG_INFO, "OpenGL Renderer {c}", glGetString(GL_RENDERER));
    LogFormat(&state->log, LOG_INFO, "OpenGL Vendor {c}", glGetString(GL_VENDOR));
    LogFormat(&state->log, LOG_INFO, "OpenGL GLSL {c}", glGetString(GL_SHADING_LANGUAGE_VERSION));
#endif

    ResetSettings(&state->settings);
    NewProject(&state->project);
    NewMap(&state->map);
    HandleArguments(argc, argv, state);

    if(!InitEditor(state)) return EXIT_FAILURE;

    tc_init(&state->textures);
    if(state->project.file.size > 0)
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
            glClearNamedFramebufferfv(state->gl.editorFramebufferMS, GL_COLOR, 0, state->settings.colors[COL_BACKGROUND]);
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
            glClearNamedFramebufferfv(state->gl.realtimeFramebuffer, GL_COLOR, 0, state->settings.colors[COL_RTBACKGROUND]);
            const float depth = 1.0f;
            glClearNamedFramebufferfv(state->gl.realtimeFramebuffer, GL_DEPTH, 0, &depth);
            RenderRealtimeView(state);

            glDisable(GL_CULL_FACE);
            glDisable(GL_DEPTH_TEST);
        }

        igRender();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, (int)ioptr->DisplaySize.x, (int)ioptr->DisplaySize.y);
        glClearNamedFramebufferfv(0, GL_COLOR, 0, state->settings.colors[COL_WORKSPACE_BACK]);
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
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

    //SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
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

    uint8_t *font = malloc(gFontSize); // imgui needs ownership of the font and the embedded data cant be free'd at imgui destruction time. should probably free it at the end but eh
    memcpy(font, gFontData, gFontSize);
    ImFontAtlas_AddFontFromMemoryTTF(ioptr->Fonts, font, gFontSize, 15, NULL, NULL); // why does imgui take ownership of the font ??? or does it??

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
