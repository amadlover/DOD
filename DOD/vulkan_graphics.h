#pragma once

#include "error.h"
#include "types.h"

#include <Windows.h>

AGE_RESULT graphics_init (const size_t game_current_max_asteroid_count, const size_t game_asteroid_live_count, const size_t game_current_max_bullet_count, const size_t game_bullet_live_count);
AGE_RESULT graphics_update_command_buffers (const size_t game_asteroid_live_count, const size_t game_bullet_live_count);
AGE_RESULT graphics_create_transforms_buffer (const size_t game_current_max_asteroid_count);
AGE_RESULT graphics_update_transforms_buffer_data (const actor_transform_outputs* game_player_transform_outputs, const actor_transform_outputs* game_asteroids_transform_outputs, const size_t game_asteroid_live_count, const actor_transform_outputs* game_bullets_transform_outputs, const size_t game_bullet_live_count);
AGE_RESULT graphics_submit_present (void);
void graphics_shutdown (void);