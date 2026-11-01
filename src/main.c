int EditorMain(int argc, char *argv[]);

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <stdlib.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    //int argc = __argc;
    //char **argv = __argv;
    int argc = *__p___argc();
    char **argv = *__p___argv();
#else
int main(int argc, char *argv[])
{
#endif
    return EditorMain(argc, argv);
}
