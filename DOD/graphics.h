#pragma once

#include "error.h"
#include "math_types.h"

#include <Windows.h>

AGE_RESULT graphics_init (HINSTANCE h_instance, HWND h_wnd, const vec2** actor_positions, const size_t* actor_count);
AGE_RESULT graphics_draw_background (void);
AGE_RESULT graphics_submit_present (void);
void graphics_exit (void);


void graphics_check_data_from_game ();