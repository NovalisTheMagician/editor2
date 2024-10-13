#include "../gwindows.h"

void AboutWindow(bool *p_open)
{
    if(igBegin("About", p_open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking))
    {
        igText("Editor2\nA map editor for the WeekRPG project\nMade by Novalis");
    }
    igEnd();
}
