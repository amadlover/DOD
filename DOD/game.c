#include "game.h"


float* actors_positions = NULL;
float* actors_rotations = NULL;
float* actors_scales = NULL;

size_t memory_reserved_for_actors = 0;

size_t num_actors = 0;

void game_init ()
{
}

void game_reserve_memory_for_actors (size_t num_actors)
{
    memory_reserved_for_actors = num_actors;
    
    actors_positions = (float*) malloc (sizeof (float) * 2 * memory_reserved_for_actors);
    actors_rotations = (float*) malloc (sizeof (float) * 2 * memory_reserved_for_actors);
    actors_scales = (float*) malloc (sizeof (float) * 2 * memory_reserved_for_actors);
}


void game_add_actor ()
{
}

void game_update ()
{
}

void game_exit ()
{
    free (actors_positions);
    free (actors_rotations);
    free (actors_scales);
}
