#pragma once

#include "error.h"
#include "types.h"

#include <Windows.h>

AGE_RESULT graphics_common_graphics_init (
    HINSTANCE h_instance, 
    HWND h_wnd, 
    const actor_transform_outputs** game_actors_transform_outputs, 
    const actor_transform_outputs* game_player_trasnform_outputs,
    const size_t* game_actor_count, 
    const size_t* game_current_max_actor_count, 
    const size_t* game_ACTOR_BATCH_SIZE
);
AGE_RESULT graphics_init (void);
AGE_RESULT graphics_update_command_buffers (void);
AGE_RESULT graphics_create_transforms_buffer (void);
AGE_RESULT graphics_update_transforms_buffer (void);
AGE_RESULT graphics_submit_present (void);
void graphics_shutdown (void);


void graphics_check_data_from_game (void);
