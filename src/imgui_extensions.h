#pragma once

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>

#include "common.h"

bool igInputText_pstr(const char *label, pstring *buf, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void *user_data);
