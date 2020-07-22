#include "game.h"
#include "graphics.h"
#include "math_types.h"

#include <stdlib.h>
#include <time.h>
#include <stdio.h>

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

vec2* actors_positions = NULL; // x,y positions clamped between -100 and +100
vec2* actors_directions = NULL; // x,y normalized vectors
vec2* actors_rotations = NULL;  //  x,y rotations, rotation_speeds

size_t memory_reserved_for_actors = 0;
size_t actor_count = 0;

void game_reserve_memory_for_actors (const size_t actors_to_reserve)
{
    memory_reserved_for_actors = actors_to_reserve;
    
    actors_positions = (vec2*) calloc (memory_reserved_for_actors, sizeof (vec2));
    actors_directions = (vec2*) calloc (memory_reserved_for_actors, sizeof (vec2));
    actors_rotations = (vec2*) calloc (memory_reserved_for_actors, sizeof (vec2));
}

AGE_RESULT game_init (const HINSTANCE h_instance, const HWND h_wnd)
{
    game_reserve_memory_for_actors (5);
    srand (time (NULL));
    
    AGE_RESULT result = graphics_init (h_instance, h_wnd, &actors_positions, &actor_count);

    if (result != AGE_SUCCESS)
    {
        goto exit;
    }

exit:  // place to clean up local allocations
    return result;
}

AGE_RESULT game_add_actor (size_t x, size_t y)
{
    if (actor_count == memory_reserved_for_actors)
    {
        memory_reserved_for_actors += 5;
        
        vec2* temp = (vec2*)realloc (actors_positions, sizeof (vec2) * memory_reserved_for_actors);
        if (temp != NULL)
        {
            actors_positions = temp;
        }
        else
        {
            printf ("Could not realloc %d bytes for game_actors_positions\n", sizeof (vec2) * memory_reserved_for_actors);
        }
        
        temp = (vec2*)realloc (actors_directions, sizeof (vec2) * memory_reserved_for_actors);

        if (temp != NULL)
        {
            actors_directions = temp;
        }
        else
        {
            printf ("Could not realloc %d bytes for actor_directions\n", sizeof (vec2) * memory_reserved_for_actors);
        }
        
        temp = (vec2*)realloc (actors_rotations, sizeof (vec2) * memory_reserved_for_actors);

        if (temp != NULL)
        {
            actors_rotations = temp;
        }
        else
        {
            printf ("Could not realloc %d bytes for actor_rotations\n", sizeof (vec2) * memory_reserved_for_actors);
        }
    }
    
    actors_positions[actor_count].x = ((float)x / (float)640) * 200.f - 100.f;
    actors_positions[actor_count].y = ((float)y / (float)480) * 200.f - 100.f;

    actors_directions[actor_count].x = (float)rand () / (float)RAND_MAX;
    actors_directions[actor_count].y = (float)rand () / (float)RAND_MAX;
    
    actors_rotations[actor_count].x = (float)rand () / (float)RAND_MAX * 360.f;
    actors_rotations[actor_count].y = (float)rand () / (float)RAND_MAX * 10.f;

    ++actor_count;

    printf ("GAME\n");
    for (size_t n = 0; n < actor_count; ++n)
    {
        printf ("Positions n = %d, x = %f, y = %f\n", n, actors_positions[n].x, actors_positions[n].y);
        printf ("Directions n = %d, x = %f, y = %f\n", n, actors_directions[n].x, actors_directions[n].y);
        printf ("Rotations n = %d, r = %f, s = %f\n", n, actors_rotations[n].x,  actors_rotations[n].y);
    }

    graphics_check_data_from_game ();
    
    return AGE_SUCCESS;
}

AGE_RESULT game_process_left_mouse_click (const size_t x, const size_t y)
{
    printf ("Left click at %d %d\n", x, y);
    AGE_RESULT age_result = AGE_SUCCESS;

    age_result = game_add_actor (x, y);
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }
    
    age_result = graphics_update_command_buffers ();
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

exit:
    return age_result;
}

void game_process_right_mouse_click (const size_t x, const size_t y)
{
    printf ("Right click at %d %d\n", x, y);
}

AGE_RESULT game_update (void)
{
    //printf ("game_update\n");
    
    for (size_t n = 0; n < actor_count; ++n)
    {
        actors_rotations[n].x += actors_rotations[n].y;
    }

    AGE_RESULT age_result = AGE_SUCCESS;

    age_result = graphics_submit_present ();

exit:
    return age_result;
}

void game_exit (void)
{
    free (actors_positions);
    free (actors_directions);
    free (actors_rotations);

    graphics_exit ();
}
