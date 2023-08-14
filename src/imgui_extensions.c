#include "imgui_extensions.h"

bool igInputText_pstr(const char *label, pstring *buf, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void *user_data)
{
    bool res = igInputText(label, buf->data, buf->capacity, flags, callback, user_data);
    if(res)
        buf->size = strlen(pstr_tocstr(*buf));
    return res;
}
