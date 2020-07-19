#pragma once

#include <stdint.h>
#include <Windows.h>

int game_init ();
void game_process_left_mouse_click (size_t x, size_t y);
void game_process_right_mouse_click (size_t x, size_t y);
void game_update ();
void game_exit ();
