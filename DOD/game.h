#pragma once

#include "error.h"

#include <stdint.h>
#include <Windows.h>

AGE_RESULT game_init (const HINSTANCE h_instance, const HWND h_wnd);
AGE_RESULT game_process_left_mouse_click (const int32_t x, const int32_t y);
AGE_RESULT game_process_right_mouse_click (const int32_t x, const int32_t y);
AGE_RESULT game_process_mouse_move (const int32_t x, const int32_t y);
AGE_RESULT game_update (void);
AGE_RESULT game_submit_present (void);
void game_exit (void);
