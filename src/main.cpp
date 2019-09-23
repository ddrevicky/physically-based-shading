#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cassert>
#ifdef _WIN32
    #include <windows.h>
    #include <io.h>
    #ifdef _DEBUG
        #define _CRTDBG_MAP_ALLOC
        #include <crtdbg.h>
    #endif
#endif
#ifdef unix
    #include <unistd.h>
#endif

#include <glad/glad.h> 
#include <SDL2/SDL.h>

#include "imgui.h"
#include "imgui_impl_sdl_gl3.h"

#include "App.h"

#define CHECK_SDL(x) \
do { \
  if ((x) != 0) { \
    std::cerr << "Error setting SDL attribute: " << SDL_GetError() << "\n"; \
    return false; } \
} while (0)

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

bool gIsRunning = true;

bool InitSDLWindow(SDL_Window **window, Uint32 *windowID)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1));
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4));

    *window = SDL_CreateWindow("PBR", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                               SCREEN_WIDTH, SCREEN_HEIGHT, 
                               SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_ALWAYS_ON_TOP);
    if (*window == nullptr)
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    *windowID = SDL_GetWindowID(*window);
    return true;
}

#ifdef _WIN32
    void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message,const void *userParam)
#else
    void glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message,const void *userParam)
#endif
{
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::cerr << "---------------" << std::endl;
    std::cerr << "Debug message (" << id << "): " << message << std::endl;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             std::cerr << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cerr << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cerr << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cerr << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     std::cerr << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           std::cerr << "Source: Other"; break;
    } std::cerr << std::endl;

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               std::cerr << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cerr << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cerr << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         std::cerr << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         std::cerr << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              std::cerr << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          std::cerr << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           std::cerr << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               std::cerr << "Type: Other"; break;
    } std::cerr << std::endl;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         std::cerr << "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       std::cerr << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          std::cerr << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: std::cerr << "Severity: notification"; break;
    } std::cerr << std::endl;
    std::cerr << std::endl;
}

bool InitSDLOpenGL(SDL_Window *window, SDL_GLContext *glContext)
{
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE));
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1));
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4));
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5));

#ifdef _DEBUG
    CHECK_SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG));
#endif
    
    *glContext = SDL_GL_CreateContext(window);
    if (!(*glContext))
    {
        std::cerr << "Error creating OpenGL context: SDL Error: " << SDL_GetError() << "\n";
        return false;
    }

    // Use v-sync
    CHECK_SDL(SDL_GL_SetSwapInterval(1));
    
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    ImGui_ImplSdlGL3_Init(window);

#ifdef _DEBUG
    // OpenGL Debug context
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(glDebugOutput, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
#endif

    return true;
}

void SDLRelease(SDL_Window *window, SDL_GLContext glContext)
{
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void onWindowResized(int w, int h)
{
}

void onMouseMove(SDL_Event &event, UserInput *input, unsigned int windowID)
{
    if (event.window.windowID == windowID && (event.motion.state & SDL_BUTTON_LMASK))
    {
        input->mouseRelX += event.motion.xrel;
        input->mouseRelY += event.motion.yrel;
    }
}

void onMouseDown(Uint8 button, unsigned /*x*/, unsigned /*y*/)
{
    if (button == SDL_BUTTON_LEFT)
    {
        SDL_ShowCursor(SDL_DISABLE);
    }
}

void onMouseWheel(Sint32 mouseWheel, UserInput *input)
{
    input->mouseWheel += mouseWheel;
}

void onMouseUp(Uint8 button, unsigned /*x*/, unsigned /*y*/)
{
    if (button == SDL_BUTTON_LEFT)
    {
        SDL_ShowCursor(SDL_ENABLE);
    }
}

void onKeyDown(SDL_Keycode key, UserInput *input)
{
    switch (key) 
    {
        case SDLK_ESCAPE: 
            gIsRunning = false; 
            return;
        case SDLK_UP:
            break;
        case SDLK_DOWN:
            break;
        case SDLK_1:
            break;
        case SDLK_w:
            input->wPressed = true;
            break;
        case SDLK_s:
            input->sPressed = true;
            break;
        case SDLK_q:
            input->qPressed = true;
            break;
        default:
            break;
    }
}

void onKeyUp(SDL_Keycode key, UserInput *input)
{
    switch (key) 
    {
        case SDLK_UP:
            break;
        case SDLK_DOWN:
            break;
        case SDLK_1:
            break;
        case SDLK_w:
            input->wPressed = false;
            break;
        case SDLK_s:
            input->sPressed = false;
            break;
        case SDLK_q:
            input->qPressed = false;
            break;
        default:
            break;
    }
}

#define MAXSAMPLES 100
int tickindex = 0;
float ticksum = 0;
float ticklist[MAXSAMPLES] = {};

float CalcAverageTick(float newtick)
{
    ticksum -= ticklist[tickindex];
    ticksum += newtick;
    ticklist[tickindex] = newtick;
    if (++tickindex == MAXSAMPLES)
        tickindex = 0;

    return(ticksum / MAXSAMPLES);
}

#undef main		// Necessary because of SDL on Windows
int main()
{
#if defined(_WIN32) && defined(_DEBUG)
    unsigned int crtFlags = _CRTDBG_LEAK_CHECK_DF;	// Perform automatic leak checking at program exit through a call to _CrtDumpMemoryLeaks
    crtFlags |= _CRTDBG_DELAY_FREE_MEM_DF;			// Keep freed memory blocks in the heap's linked list, assign them the _FREE_BLOCK type, and fill them with the byte value 0xDD
    crtFlags |= _CRTDBG_CHECK_ALWAYS_DF;			// Call _CrtCheckMemory at every allocation and deallocation request. _crtBreakAlloc = 323; tracks the erroneous malloc
    _CrtSetDbgFlag(crtFlags);
#endif

    SDL_Window *window = nullptr;
    Uint32 windowID = 0;
    SDL_GLContext glContext;    

    if (!InitSDLWindow(&window, &windowID) || !InitSDLOpenGL(window, &glContext))
    {
        std::cerr << "Initialization failed\n";
        return -1;
    }

    AppContext appContext;
    App::Init(&appContext, SCREEN_WIDTH, SCREEN_HEIGHT);

    Uint64 currentTick = SDL_GetPerformanceCounter();
    Uint64 previousTick = 0;
    Uint64 performanceFrequency = SDL_GetPerformanceFrequency();
    while (gIsRunning)
    {   
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSdlGL3_ProcessEvent(&event);

            switch (event.type)
            {
                case SDL_KEYDOWN:
                    onKeyDown(event.key.keysym.sym, &appContext.userInput);
                    break;
                case SDL_KEYUP:
                    onKeyUp(event.key.keysym.sym, &appContext.userInput);
                    break;
                case SDL_MOUSEMOTION:
                    onMouseMove(event, &appContext.userInput, windowID);
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    onMouseDown(event.button.button, event.button.x, event.button.y);
                    break;
                case SDL_MOUSEBUTTONUP:
                    onMouseUp(event.button.button, event.button.x, event.button.y);
                    break;
                case SDL_MOUSEWHEEL:
                    onMouseWheel(event.wheel.y, &appContext.userInput);
                    break;
                case SDL_QUIT:
                    gIsRunning = false;
                    break;
                case SDL_WINDOWEVENT:
                    switch (event.window.event)
                    {
                        case SDL_WINDOWEVENT_RESIZED:
                            onWindowResized(event.window.data1, event.window.data2);
                            break;
                        default:
                            break;
                    }
                default:
                    break;
            }
        }

        previousTick = currentTick;
        currentTick = SDL_GetPerformanceCounter();
        double dt = (currentTick - previousTick) / double(performanceFrequency);
#if 0
		char debugBuffer[1000];
        double avgFPS = 1.0f / CalcAverageTick(dt);
        double avgMSPerFrame = 1000 * (1.0 / avgFPS);
        sprintf_s(debugBuffer, "%.2lf msPerFrame %.2lf \n", avgFPS, avgMSPerFrame);
        OutputDebugString(debugBuffer);
#endif

        ImGui_ImplSdlGL3_NewFrame(window);

        App::Update(&appContext, dt);

        ImGui::Render();
        SDL_GL_SwapWindow(window);
    }
    App::Release(&appContext);
    SDLRelease(window, glContext);

#if defined(_WIN32) && defined(_DEBUG)
    _CrtDumpMemoryLeaks();
#endif

    return 0;
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    return main();
}
#endif
