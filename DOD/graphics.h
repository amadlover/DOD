#pragma once

#include "error.h"
#include <Windows.h>

AGE_RESULT graphics_init (HINSTANCE h_instance, HWND h_wnd);
AGE_RESULT graphics_draw_background (void);
AGE_RESULT graphics_submit_present (void);
void graphics_exit (void);
