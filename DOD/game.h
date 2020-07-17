#pragma once

#include <stdint.h>

void game_init ();
void game_reserve_memory_for_actors (size_t num_actors);
void game_add_actor ();
void game_update ();
void game_exit ();
