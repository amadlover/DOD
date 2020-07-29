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

vec2* game_actors_positions = NULL; // x,y positions clamped between -100 and +100
vec2* game_actors_directions = NULL; // x,y normalized vectors
vec2* game_actors_rotations = NULL;  //  x,y rotations, rotation_speeds

size_t game_current_max_actor_count = 0;
size_t game_actor_count = 0;
const size_t game_ACTOR_BATCH_SIZE = 50;

AGE_RESULT game_reserve_memory_for_actors ()
{
    AGE_RESULT age_result = AGE_SUCCESS;

    game_current_max_actor_count += game_ACTOR_BATCH_SIZE;
    
    game_actors_positions = (vec2*) calloc (game_current_max_actor_count, sizeof (vec2));
    game_actors_directions = (vec2*) calloc (game_current_max_actor_count, sizeof (vec2));
    game_actors_rotations = (vec2*) calloc (game_current_max_actor_count, sizeof (vec2));


exit: // clean up allocations done in this function

    return age_result;
}

AGE_RESULT game_init (const HINSTANCE h_instance, const HWND h_wnd)
{
    AGE_RESULT age_result = AGE_SUCCESS;

    srand (time (NULL));

    age_result = game_reserve_memory_for_actors ();
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

    age_result = graphics_common_graphics_init (
        h_instance, 
        h_wnd,         
        &game_actors_positions, 
        &game_actor_count, 
        &game_current_max_actor_count, 
        &game_ACTOR_BATCH_SIZE
    );

    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

    age_result = graphics_create_transforms_buffer ();
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

    age_result = graphics_init (

    );

    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

exit:  // place to clean up local allocations
    return age_result;
}

AGE_RESULT game_add_actor (size_t x, size_t y)
{
    AGE_RESULT age_result = AGE_SUCCESS;

    if (game_actor_count == game_current_max_actor_count)
    {
        game_current_max_actor_count += game_ACTOR_BATCH_SIZE;

        vec2* temp = (vec2*)realloc (game_actors_positions, sizeof (vec2) * game_current_max_actor_count);
        if (temp != NULL)
        {
            game_actors_positions = temp;
        }
        else
        {
            printf ("Could not realloc %d bytes for game_actors_transforms\n", sizeof (vec2) * game_current_max_actor_count);
        }

        temp = (vec2*)realloc (game_actors_directions, sizeof (vec2) * game_current_max_actor_count);

        if (temp != NULL)
        {
            game_actors_directions = temp;
        }
        else
        {
            printf ("Could not realloc %d bytes for actor_directions\n", sizeof (vec2) * game_current_max_actor_count);
        }

        temp = (vec2*)realloc (game_actors_rotations, sizeof (vec2) * game_current_max_actor_count);

        if (temp != NULL)
        {
            game_actors_rotations = temp;
        }
        else
        {
            printf ("Could not realloc %d bytes for actor_rotations\n", sizeof (vec2) * game_current_max_actor_count);
        }

        age_result = graphics_create_transforms_buffer ();
        if (age_result != AGE_SUCCESS)
        {
            goto exit;
        }
    }


    game_actors_positions[game_actor_count].x = ((float)rand () / (float)RAND_MAX) * 2 - 1;
    game_actors_positions[game_actor_count].y = ((float)rand () / (float)RAND_MAX) * 2 - 1;

    game_actors_directions[game_actor_count].x = (float)rand () / (float)RAND_MAX;
    game_actors_directions[game_actor_count].y = (float)rand () / (float)RAND_MAX;
    
    game_actors_rotations[game_actor_count].x = (float)rand () / (float)RAND_MAX * 360.f;
    game_actors_rotations[game_actor_count].y = (float)rand () / (float)RAND_MAX * 10.f;

    ++game_actor_count;

    printf ("GAME\n");
    for (size_t n = 0; n < game_actor_count; ++n)
    {
        printf ("Positions n = %d, x = %f, y = %f\n", n, game_actors_positions[n].x, game_actors_positions[n].y);
    }

    printf ("current max actors %d, actor count %d ACTOR BATCH SIZE %d\n", game_current_max_actor_count, game_actor_count, game_ACTOR_BATCH_SIZE);

    graphics_check_data_from_game ();
    
exit:
    return age_result;;
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
    
    AGE_RESULT age_result = AGE_SUCCESS;

    age_result = graphics_update_transforms_buffer();
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

    age_result = graphics_submit_present ();
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

exit: // clear function specific allocations
    return age_result;
}

void game_exit (void)
{
    free (game_actors_positions);
    free (game_actors_directions);
    free (game_actors_rotations);

    graphics_exit ();
}
