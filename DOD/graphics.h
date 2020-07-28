#pragma once

#include "error.h"
#include "math_types.h"

#include <Windows.h>

AGE_RESULT graphics_init (HINSTANCE h_instance, HWND h_wnd, const vec2** game_actor_positions, const size_t* game_actor_count, const size_t* game_current_max_actor_count, const size_t* game_ACTOR_BATCH_SIZE);
AGE_RESULT graphics_update_command_buffers (void);
AGE_RESULT graphics_update_transforms_buffer (void);
AGE_RESULT graphics_submit_present (void);
void graphics_exit (void);


void graphics_check_data_from_game (void);