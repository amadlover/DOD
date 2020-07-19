#pragma once

#include "error.h"
#include <stdint.h>
#include <Windows.h>

AGE_RESULT game_init (HINSTANCE h_instance, HWND h_wnd);
void game_process_left_mouse_click (size_t x, size_t y);
void game_process_right_mouse_click (size_t x, size_t y);
void game_update ();
void game_exit ();
