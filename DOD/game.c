#include "game.h"

typedef struct vec2_
{
    float x;
    float y;
} vec2;

vec2* actors_positions = NULL; // x,y positions

size_t memory_reserved_for_actors = 0;

size_t num_actors = 0;

void game_reserve_memory_for_actors (size_t actors_to_reserve)
{
    memory_reserved_for_actors = actors_to_reserve;
    
    actors_positions = (vec2*) calloc (memory_reserved_for_actors, sizeof (vec2));
}

void game_init ()
{
    game_reserve_memory_for_actors (5);
}

void game_add_actor (size_t x, size_t y)
{
    if ((num_actors + 1) > memory_reserved_for_actors)
    {
        memory_reserved_for_actors += 5;
        actors_positions = (vec2*) realloc (actors_positions, sizeof (vec2) * memory_reserved_for_actors);

    }
    
    printf ("%d\n", num_actors);
    actors_positions[num_actors].x = x;
    actors_positions[num_actors].y = y;
    
    ++num_actors;

    printf ("%d\n", num_actors);

    for (size_t n = 0; n < num_actors; ++n)
    {
        printf ("n = %d, x = %f, y = %f\n", n, actors_positions[n].x, actors_positions[n].y);
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
    for (size_t n = 0; n < num_actors; ++n)
    {
        actors_positions[n].x += (float)n / 100.f;
        actors_positions[n].y += (float)n / 100.f;
    }
}

void game_exit ()
{
    free (actors_positions);
}
