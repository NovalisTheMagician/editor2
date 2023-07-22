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

#include <incbin.h>
INCBIN_EXTERN(Font);

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
    SetStyle(state.settings.theme);

    if(!InitEditor(&state)) return EXIT_FAILURE;

    struct Map map = { 0 };
    NewMap(&map);

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

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        igNewFrame();

        if(DoGui(&state, &map))
            quit = true;

        if(state.gl.editorColorTexture > 0)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, state.gl.editorFramebuffer);
            glViewport(0, 0, state.gl.editorFramebufferWidth, state.gl.editorFramebufferHeight);
            glClearColor(state.settings.colors[COL_BACKGROUND][0], state.settings.colors[COL_BACKGROUND][1], state.settings.colors[COL_BACKGROUND][2], state.settings.colors[COL_BACKGROUND][3]);
            glClear(GL_COLOR_BUFFER_BIT);
            RenderEditorView(&state, &map);
        }

        if(state.ui.show3dView && state.gl.realtimeColorTexture > 0)
        {
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);

            glBindFramebuffer(GL_FRAMEBUFFER, state.gl.realtimeFramebuffer);
            glViewport(0, 0, state.gl.realtimeFramebufferWidth, state.gl.realtimeFramebufferHeight);
            glClearColor(state.settings.colors[COL_RTBACKGROUND][0], state.settings.colors[COL_RTBACKGROUND][1], state.settings.colors[COL_RTBACKGROUND][2], state.settings.colors[COL_RTBACKGROUND][3]);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            RenderRealtimeView(&state, &map);

            glDisable(GL_CULL_FACE);
            glDisable(GL_DEPTH_TEST);
        }

        igRender();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, (int)ioptr->DisplaySize.x, (int)ioptr->DisplaySize.y);
        glClearColor(state.settings.colors[COL_WORKSPACE_BACK][0], state.settings.colors[COL_WORKSPACE_BACK][1], state.settings.colors[COL_WORKSPACE_BACK][2], state.settings.colors[COL_WORKSPACE_BACK][3]);
        glClear(GL_COLOR_BUFFER_BIT);
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
        const char *errMsg = SDL_GetError();
        printf("failed to init sdl: %s\n", errMsg);
        return NULL;
    }
#ifdef _DEBUG
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);

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

    uint8_t *font = malloc(gFontSize); // why do we have to make a copy here? should probably free it at the end but eh
    memcpy(font, gFontData, gFontSize);
    ImFontAtlas_AddFontFromMemoryTTF(ioptr->Fonts, font, gFontSize, 15, NULL, NULL); // why does imgui take ownership of the font ???

    ioptr->IniFilename = NULL;

    if(!ImGui_ImplSDL2_InitForOpenGL(window, context))
        return false;
    if(!ImGui_ImplOpenGL3_Init("#version 460"))
        return false;

    return true;
}

static SDL_GLContext InitOpenGL(SDL_Window *window)
{
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(1);

    if(SDL_GL_MakeCurrent(window, glContext) < 0)
    {
        const char *errMsg = SDL_GetError();
        printf("failed to make context current: %s\n", errMsg);
        return false;
    }

    if(!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress))
    {
        GLenum err = glGetError();
        printf("couldn't load GL functions: %d\n", err);
        return NULL;
    }

    return glContext;
}
