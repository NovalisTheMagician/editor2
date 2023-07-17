#include "common.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_USE_OPENGL3
#define CIMGUI_USE_SDL2
#include <cimgui.h>
#include <cimgui_impl.h>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "editor.h"
#include "map.h"
#include "gui.h"

#define SETTINGS_FILE "./settings.ini"
#define DEFAULT_WINDOW_WIDTH 1600
#define DEFAULT_WINDOW_HEIGHT 900

static SDL_Window* InitSDL(void);
static bool InitImgui(SDL_Window *window, SDL_GLContext context);
static SDL_GLContext InitOpenGL(SDL_Window *window);

int EditorMain(int argc, char *argv[])
{
    atexit(SDL_Quit);

    struct EdState state = { 0 };
    if(!LoadSettings(SETTINGS_FILE, &state.settings))
    {
        printf("failed to load settings from %s!\nusing default values\n", SETTINGS_FILE);
        ResetSettings(&state.settings);
    }

    SDL_Window *window = InitSDL();
    if(!window) return EXIT_FAILURE;

    SDL_GLContext glContext = InitOpenGL(window);
    if(!glContext) return EXIT_FAILURE;

    if(!InitImgui(window, glContext)) return EXIT_FAILURE;

    if(!InitEditor(&state)) return EXIT_FAILURE;

    struct Map map = { 0 };

    ImGuiIO *ioptr = igGetIO();

    SDL_Event e;
    bool quit = false;
    while(!quit)
    {
        while(SDL_PollEvent(&e) > 0)
        {
            ImGui_ImplSDL2_ProcessEvent(&e);
            //if(!igIsWindowFocused(ImGuiFocusedFlags_AnyWindow))
            //if(!ioptr->WantCaptureKeyboard && !ioptr->WantCaptureMouse)
            //    HandleInputEvents(&e, &state, &map);

            switch(e.type)
            {
            case SDL_QUIT:
                quit = true;
                break;
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        igNewFrame();

        if(DoGui(&state, &map))
            quit = true;

        glBindFramebuffer(GL_FRAMEBUFFER, state.editorFramebuffer);
        glViewport(0, 0, state.editorFramebufferWidth, state.editorFramebufferHeight);
        glClearColor(0, 1, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        RenderEditor(&state, &map);

        igRender();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, (int)ioptr->DisplaySize.x, (int)ioptr->DisplaySize.y);
        glClearColor(state.settings.colors[BACKGROUND][0], state.settings.colors[BACKGROUND][1], state.settings.colors[BACKGROUND][2], state.settings.colors[BACKGROUND][3]);
        glClear(GL_COLOR_BUFFER_BIT);

        //RenderEditor(&state, &map);

        ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());

        SDL_GL_SwapWindow(window);
    }

    SaveSettings(SETTINGS_FILE, &state.settings);

    DestroyEditor(&state);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    igDestroyContext(NULL);

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);

    return EXIT_SUCCESS;
}

static SDL_Window* InitSDL(void)
{
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        printf("failed to init sdl\n");
        return NULL;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    uint32_t flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
#ifndef _DEBUG
    flags |= SDL_WINDOW_MAXIMIZED;
#endif

    SDL_Window *window = SDL_CreateWindow("Editor", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, flags);
    if(!window)
    {
        printf("failed to create window\n");
        return NULL;
    }

    return window;
}

static bool InitImgui(SDL_Window *window, SDL_GLContext context)
{
    igCreateContext(NULL);

    ImGuiIO *ioptr = igGetIO();
    ioptr->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    //ioptr->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ioptr->IniFilename = NULL;

    ImGui_ImplSDL2_InitForOpenGL(window, context);
    ImGui_ImplOpenGL3_Init("#version 460");

    //igStyleColorsDark(NULL);
    igStyleColorsLight(NULL);
    //SetStyle(igGetStyle());

    return true;
}

static SDL_GLContext InitOpenGL(SDL_Window *window)
{
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(1);

    if(!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress))
    {
        printf("couldn't load GL functions");
        return NULL;
    }

    SDL_GL_MakeCurrent(window, glContext);
    return glContext;
}
