#pragma once

#include "error.h"
#include <stdint.h>
#include <Windows.h>

AGE_RESULT game_init (const HINSTANCE h_instance, const HWND h_wnd);
AGE_RESULT game_process_left_mouse_click (const size_t x, const size_t y);
void game_process_right_mouse_click (const size_t x, const size_t y);
AGE_RESULT game_update (void);
void game_exit (void);
