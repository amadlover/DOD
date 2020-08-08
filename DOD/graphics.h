#pragma once

#include "error.h"
#include "types.h"

#include <Windows.h>

AGE_RESULT graphics_create_descriptor_sets (void);
AGE_RESULT graphics_init (void);
AGE_RESULT graphics_update_command_buffers (const size_t game_live_actor_count);
AGE_RESULT graphics_create_transforms_buffer (const size_t game_current_max_actor_count);
AGE_RESULT graphics_update_transforms_buffer_data (const actor_transform_outputs* game_player_transform_outputs, const actor_transform_outputs* game_actors_transform_outputs, const size_t game_live_actor_count);
AGE_RESULT graphics_submit_present (void);
void graphics_shutdown (void);