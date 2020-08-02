#include "game.h"
#include "graphics.h"
#include "types.h"

#include "utils.h"

#include <stdlib.h>
#include <stdio.h>

/*
 * OOP struct
 *
 * struct actor {
 *     vec2 position_inputs;
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


actor_transform_inputs* game_actors_transform_inputs = NULL;
actor_transform_outputs* game_actors_transform_outputs = NULL;

size_t game_current_max_actor_count = 0;
size_t game_live_actor_count = 0;
const size_t game_ACTOR_BATCH_SIZE = 50;

AGE_RESULT game_reserve_memory_for_actors ()
{
    AGE_RESULT age_result = AGE_SUCCESS;

    game_current_max_actor_count += game_ACTOR_BATCH_SIZE;

    game_actors_transform_inputs = (actor_transform_inputs*)utils_calloc (game_current_max_actor_count, sizeof (actor_transform_inputs));
    game_actors_transform_outputs = (actor_transform_outputs*)utils_calloc (game_current_max_actor_count, sizeof (actor_transform_outputs));

exit: // clean up allocations done in this function

    return age_result;
}

AGE_RESULT game_init (const HINSTANCE h_instance, const HWND h_wnd)
{
    AGE_RESULT age_result = AGE_SUCCESS;

    srand (rand ());

    age_result = game_reserve_memory_for_actors ();
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

    age_result = graphics_common_graphics_init (
        h_instance,
        h_wnd,
        &game_actors_transform_outputs,
        &game_live_actor_count,
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

    age_result = graphics_init ();

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

    if (game_live_actor_count == game_current_max_actor_count)
    {
        game_current_max_actor_count += game_ACTOR_BATCH_SIZE;

        game_actors_transform_inputs = (actor_transform_inputs*) utils_realloc (game_actors_transform_inputs, sizeof (actor_transform_inputs) * game_current_max_actor_count);
        game_actors_transform_outputs = (actor_transform_outputs*) utils_realloc (game_actors_transform_outputs, sizeof (actor_transform_outputs) * game_current_max_actor_count);

        age_result = graphics_create_transforms_buffer ();
        if (age_result != AGE_SUCCESS)
        {
            goto exit;
        }
    }

    game_actors_transform_outputs[game_live_actor_count].position.x = ((float)rand () / (float)RAND_MAX) * 2 - 1;
    game_actors_transform_outputs[game_live_actor_count].position.y = ((float)rand () / (float)RAND_MAX) * 2 - 1;
    game_actors_transform_outputs[game_live_actor_count].rotation = (float)rand () / (float)RAND_MAX * 360.f;

    game_actors_transform_inputs[game_live_actor_count].direction.x = ((float)rand () / (float)RAND_MAX) * 2 - 1;
    game_actors_transform_inputs[game_live_actor_count].direction.y = ((float)rand () / (float)RAND_MAX) * 2 - 1;
    game_actors_transform_inputs[game_live_actor_count].position_speed = ((float)rand () / (float)RAND_MAX) / 1000.f;
    game_actors_transform_inputs[game_live_actor_count].rotation_speed = (((float)rand () / (float)RAND_MAX)) / 50.f;

    ++game_live_actor_count;

    printf ("GAME\n");
    printf ("current max actors %d, actor count %d ACTOR BATCH SIZE %d\n", game_current_max_actor_count, game_live_actor_count, game_ACTOR_BATCH_SIZE);

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
    return age_result;;
}

AGE_RESULT game_remove_actor (void)
{
    AGE_RESULT age_result = AGE_SUCCESS;

    srand (rand ());
    size_t actor_index_to_remove = (size_t)(((float)rand () / (float)RAND_MAX) * game_live_actor_count);

    printf ("index to remove: %d\n", actor_index_to_remove);

    if (game_live_actor_count > 0)
    {
        for (size_t a = actor_index_to_remove; a < game_live_actor_count; ++a)
        {
            game_actors_transform_inputs[a] = game_actors_transform_inputs[a+1];
        }

        for (size_t a = actor_index_to_remove; a < game_live_actor_count; ++a)
        {
            game_actors_transform_outputs[a] = game_actors_transform_outputs[a+1];
        }

        --game_live_actor_count;

        printf ("GAME\n");
        printf ("current max actors %d, actor count %d ACTOR BATCH SIZE %d\n", game_current_max_actor_count, game_live_actor_count, game_ACTOR_BATCH_SIZE);

        graphics_check_data_from_game ();
    }
exit:
    return age_result;
}

AGE_RESULT game_process_right_mouse_click (const size_t x, const size_t y)
{
    printf ("Right click at %d %d\n", x, y);

    AGE_RESULT age_result = AGE_SUCCESS;

    age_result = game_remove_actor ();
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

AGE_RESULT game_update (void)
{
    AGE_RESULT age_result = AGE_SUCCESS;

    for (size_t a = 0; a < game_live_actor_count; ++a)
    {
        game_actors_transform_outputs[a].position.x += (game_actors_transform_inputs[a].direction.x * game_actors_transform_inputs[a].position_speed);
        game_actors_transform_outputs[a].position.y += (game_actors_transform_inputs[a].direction.y * game_actors_transform_inputs[a].position_speed);
        game_actors_transform_outputs[a].rotation += (game_actors_transform_inputs[a].rotation_speed);
    }

    age_result = graphics_update_transforms_buffer();
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

exit: // clear function specific allocations
    return age_result;
}

AGE_RESULT game_submit_present (void)
{
    AGE_RESULT age_result = AGE_SUCCESS;

    age_result = graphics_submit_present ();
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

exit:
    return age_result;
}

void game_exit (void)
{
    graphics_exit ();

    utils_free (game_actors_transform_inputs);
    utils_free (game_actors_transform_outputs);
}