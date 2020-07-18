#include "game.h"
#include <stdlib.h>
#include <time.h>

/*
 * OOP struct
 *
 * struct actor {
 *     vec2 position;
 *     vec2 direction;
 *     vec2 rotation;
 *     vec2 scale;
 *     float positional_speed;
 *     float rotational_speed;
 *     char name[256];
 *     asset* geometry;
 * }
 *
 * */

typedef struct vec2_
{
    float x;
    float y;
} vec2;

vec2* actors_positions = NULL; // x,y positions clamped between -100 and +100
vec2* actors_directions = NULL; // x,y normalized vectors
float* actors_rotations = NULL; // float value

size_t memory_reserved_for_actors = 0;
size_t num_actors = 0;

void game_reserve_memory_for_actors (size_t actors_to_reserve)
{
    memory_reserved_for_actors = actors_to_reserve;
    
    actors_positions = (vec2*) calloc (memory_reserved_for_actors, sizeof (vec2));
    actors_directions = (vec2*) calloc (memory_reserved_for_actors, sizeof (vec2));
    actors_rotations = (float*) calloc (memory_reserved_for_actors, sizeof (float));
}

void game_init ()
{
    game_reserve_memory_for_actors (5);
    srand (time (0));
}

void game_add_actor (size_t x, size_t y)
{
    if (num_actors == memory_reserved_for_actors)
    {
        memory_reserved_for_actors += 5;
        actors_positions = (vec2*) realloc (actors_positions, sizeof (vec2) * memory_reserved_for_actors);
        actors_directions = (vec2*) realloc (actors_directions, sizeof (vec2) * memory_reserved_for_actors);
        actors_rotations = (float*) realloc (actors_rotations, sizeof (float) * memory_reserved_for_actors);
    }
    
    actors_positions[num_actors].x = ((float)x / (float)640) * 200.f - 100.f;
    actors_positions[num_actors].y = ((float)y / (float)480) * 200.f - 100.f;

    actors_directions[num_actors].x = (float)rand () / (float)RAND_MAX;
    actors_directions[num_actors].y = (float)rand () / (float)RAND_MAX;
    
    actors_rotations[num_actors] = (float)rand () / (float)RAND_MAX * 360.f;
    
    ++num_actors;

    for (size_t n = 0; n < num_actors; ++n)
    {
        printf ("Positions n = %d, x = %f, y = %f\n", n, actors_positions[n].x, actors_positions[n].y);
        printf ("Directions n = %d, x = %f, y = %f\n", n, actors_directions[n].x, actors_directions[n].y);
        printf ("Rotations n = %d, r = %f\n", n, actors_rotations[n]);
    }
}

void game_process_left_mouse_click (size_t x, size_t y)
{
    printf ("Left click at %d %d\n", x, y);
   
    game_add_actor (x, y);
}

void game_process_right_mouse_click (size_t x, size_t y)
{
    printf ("Right click at %d %d\n", x, y);
}

void game_update ()
{
    // update the positions and rotations of the actors based on the positional speed and rotational speed
    
    for (size_t n = 0; n < num_actors; ++n)
    {
    }
}

void game_exit ()
{
    free (actors_positions);
    free (actors_directions);
    free (actors_rotations);
}
