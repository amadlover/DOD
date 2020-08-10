#include "game.h"
#include "vulkan_graphics.h"
#include "types.h"
#include "vulkan_interface.h"

#include "utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

/*
 * OOP struct
 *
 * struct actor {
 *     float2 position_inputs;
 *     float2 direction;
 *     float2 rotation;
 *     float2 scale;
 *     float positional_speed;
 *     float rotational_speed;
 *     char name[256];
 *     asset* geometry;
 * }
 *
 * */

bool is_w_pressed = false;
bool is_s_pressed = false;
bool is_d_pressed = false;
bool is_a_pressed = false;
bool is_space_bar_pressed = false;
bool is_up_arrow_pressed = false;
bool is_down_arrow_pressed = false;
bool is_right_arrow_pressed = false;
bool is_left_arrow_pressed = false;

player_transform_inputs game_player_transform_inputs = { 0 };
actor_transform_outputs game_player_transform_outputs = { 0 };

bullet_transform_inputs* game_bullets_transform_input = NULL;
actor_transform_outputs* game_bullets_transform_output = NULL;

actor_transform_inputs* game_actors_transform_inputs = NULL;
actor_transform_outputs* game_actors_transform_outputs = NULL;

size_t game_current_max_actor_count = 0;
size_t game_live_actor_count = 0;
const size_t game_ACTOR_BATCH_SIZE = 50;

RECT window_rect;
int32_t last_mouse_x;
int32_t last_mouse_y;

size_t game_delta_time = 0;


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

    game_player_transform_inputs.damping_factor = 0.975f;
    game_player_transform_inputs.forward_vector.x = 0;

    game_player_transform_inputs.forward_vector.y = 1;
    game_player_transform_inputs.acceleration = 0.00005f;
    game_player_transform_inputs.deceleration = -0.000025f;
    game_player_transform_inputs.rotation_speed = 0.075f;

    GetClientRect (h_wnd, &window_rect);

    srand (time (NULL));

    age_result = game_reserve_memory_for_actors ();
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

    age_result = vulkan_interface_init (h_instance, h_wnd);
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

    age_result = graphics_init (game_current_max_actor_count, game_live_actor_count);
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

exit:  // place to clean up local allocations
    return age_result;
}

AGE_RESULT game_add_actor (void)
{
    AGE_RESULT age_result = AGE_SUCCESS;

    if (game_live_actor_count == game_current_max_actor_count)
    {
        game_current_max_actor_count += game_ACTOR_BATCH_SIZE;

        game_actors_transform_inputs = (actor_transform_inputs*) utils_realloc (game_actors_transform_inputs, sizeof (actor_transform_inputs) * game_current_max_actor_count);
        game_actors_transform_outputs = (actor_transform_outputs*) utils_realloc (game_actors_transform_outputs, sizeof (actor_transform_outputs) * game_current_max_actor_count);

        age_result = graphics_create_transforms_buffer (game_current_max_actor_count);
        if (age_result != AGE_SUCCESS)
        {
            goto exit;
        }
    }

    srand (rand ());

    game_actors_transform_outputs[game_live_actor_count].position.x = ((float)rand () / (float)RAND_MAX) * 2 - 1;
    game_actors_transform_outputs[game_live_actor_count].position.y = ((float)rand () / (float)RAND_MAX) * 2 - 1;
    game_actors_transform_outputs[game_live_actor_count].rotation = (float)rand () / (float)RAND_MAX * 3.14f;

    game_actors_transform_inputs[game_live_actor_count].forward_vector.x = ((float)rand () / (float)RAND_MAX) * 2 - 1;
    game_actors_transform_inputs[game_live_actor_count].forward_vector.y = ((float)rand () / (float)RAND_MAX) * 2 - 1;
    game_actors_transform_inputs[game_live_actor_count].forward_speed = ((float)rand () / (float)RAND_MAX) / 500.f;
    game_actors_transform_inputs[game_live_actor_count].rotation_speed = (((float)rand () / (float)RAND_MAX)) / 50.f;

    ++game_live_actor_count;

    printf ("GAME\n");
    printf ("current max actors %d, actor count %d ACTOR BATCH SIZE %d\n", game_current_max_actor_count, game_live_actor_count, game_ACTOR_BATCH_SIZE);

exit:
    return age_result;;
}

AGE_RESULT game_process_left_mouse_click (const int32_t x, const int32_t y)
{
    printf ("Left click at %d %d\n", x, y);
    AGE_RESULT age_result = AGE_SUCCESS;

    age_result = game_add_actor ();
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }
    
    age_result = graphics_update_command_buffers (game_live_actor_count);
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

exit:
    return age_result;
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
    }

exit:
    return age_result;
}

AGE_RESULT game_process_right_mouse_click (const int32_t x, const int32_t y)
{
    printf ("Right click at %d %d\n", x, y);

    AGE_RESULT age_result = AGE_SUCCESS;

    age_result = game_remove_actor ();
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

    age_result = graphics_update_command_buffers (game_live_actor_count);
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

exit:
    return age_result;
}

AGE_RESULT game_update_player_vectors (void)
{
    AGE_RESULT age_result = AGE_SUCCESS;

    float new_vector_x = -sinf (game_player_transform_inputs.rotation);
    float new_vector_y = cosf (game_player_transform_inputs.rotation);

    game_player_transform_inputs.forward_vector.x = new_vector_x;
    game_player_transform_inputs.forward_vector.y = new_vector_y;

exit:
    return age_result;
}

AGE_RESULT game_process_mouse_move (const int32_t x, const int32_t y)
{
    AGE_RESULT age_result = AGE_SUCCESS;

    if (x < 0 || y < 0)
    {
        goto exit;
    }

    last_mouse_x = x;
    last_mouse_y = y;

exit:
    return age_result;
}

AGE_RESULT game_player_increase_speed (void)
{
    AGE_RESULT age_result = AGE_SUCCESS;

    float2 acceleration = { 
        game_player_transform_inputs.acceleration * game_player_transform_inputs.forward_vector.x, 
        game_player_transform_inputs.acceleration * game_player_transform_inputs.forward_vector.y 
    };

    game_player_transform_inputs.v.x = game_player_transform_inputs.u.x + (acceleration.x * game_delta_time);
    game_player_transform_inputs.v.y = game_player_transform_inputs.u.y + (acceleration.y * game_delta_time);

    game_player_transform_inputs.u = game_player_transform_inputs.v;

exit:
    return age_result;
}

AGE_RESULT game_player_decrease_speed (void)
{
    AGE_RESULT age_result = AGE_SUCCESS;

    float2 deceleration = { 
        game_player_transform_inputs.deceleration * game_player_transform_inputs.forward_vector.x,
        game_player_transform_inputs.deceleration * game_player_transform_inputs.forward_vector.y 
    };

    game_player_transform_inputs.v.x = game_player_transform_inputs.u.x + (deceleration.x * game_delta_time);
    game_player_transform_inputs.v.y = game_player_transform_inputs.u.y + (deceleration.y * game_delta_time);

    game_player_transform_inputs.u = game_player_transform_inputs.v;

exit:
    return age_result;
}

AGE_RESULT game_player_turn_right (void)
{
    AGE_RESULT age_result = AGE_SUCCESS;

    game_player_transform_inputs.rotation -= game_player_transform_inputs.rotation_speed;
    game_player_transform_outputs.rotation = game_player_transform_inputs.rotation;

    age_result = game_update_player_vectors ();
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

exit:
    return age_result;
}

AGE_RESULT game_player_turn_left (void)
{
    AGE_RESULT age_result = AGE_SUCCESS;
    
    game_player_transform_inputs.rotation += game_player_transform_inputs.rotation_speed;
    game_player_transform_outputs.rotation = game_player_transform_inputs.rotation;

    age_result = game_update_player_vectors ();
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }
exit:
    return age_result;
}

AGE_RESULT game_player_shoot_bullet (void)
{
    AGE_RESULT age_result = AGE_SUCCESS;
    
exit:
    return age_result;
}

AGE_RESULT game_process_key_down (const WPARAM w_param)
{
    AGE_RESULT age_result = AGE_SUCCESS;
    
    switch (w_param) 
    {
        case 0x57: // w
        is_w_pressed = true;
        break;

        case 0x53: // s
        is_s_pressed = true;
        break;

        case 0x44: // d
        is_d_pressed = true;
        break;
        
        case 0x41: // a
        is_a_pressed = true;
        break;

        case 0x26: // up arrow
        is_up_arrow_pressed = true;
        break;

        case 0x28: // down arrow
        is_down_arrow_pressed = true;
        break;

        case 0x27: // right arrow
        is_right_arrow_pressed = true;
        break;

        case 0x25: // left arrow
        is_left_arrow_pressed = true;
        break;

        case 0x20: // space
        is_space_bar_pressed = true;
        break;

        default:
        break;
    }

exit:
    return age_result;
}

AGE_RESULT game_process_key_up (const WPARAM w_param)
{
    AGE_RESULT age_result = AGE_SUCCESS;

    switch (w_param) 
    {
        case 0x57: // w
        is_w_pressed = false;
        break;

        case 0x53: // s
        is_s_pressed = false;
        break;

        case 0x44: // d
        is_d_pressed = false;
        break;
        
        case 0x41: // a
        is_a_pressed = false;
        break;

        case 0x26: // up arrow
        is_up_arrow_pressed = false;
        break;

        case 0x28: // down arrow
        is_down_arrow_pressed = false;
        break;

        case 0x27: // right arrow
        is_right_arrow_pressed = false;
        break;

        case 0x25: // left arrow
        is_left_arrow_pressed = false;
        break;

        case 0x20: // space
        is_space_bar_pressed = false;
        break;

        default:
        break;
    }

exit:
    return age_result;
}

AGE_RESULT game_update_player_actor_output_positions (void)
{
    AGE_RESULT age_result = AGE_SUCCESS;

    game_player_transform_outputs.position.x += game_player_transform_inputs.v.x;
    game_player_transform_outputs.position.y += game_player_transform_inputs.v.y;

    if (game_player_transform_outputs.position.x > 1.f)
    {
        game_player_transform_outputs.position.x = -1.f;
    }
    
    if (game_player_transform_outputs.position.x < -1.f)
    {
        game_player_transform_outputs.position.x = 1.f;
    }

    if (game_player_transform_outputs.position.y > 1.f)
    {
        game_player_transform_outputs.position.y = -1.f;
    }
    
    if (game_player_transform_outputs.position.y < -1.f)
    {
        game_player_transform_outputs.position.y = 1.f;
    }

    for (size_t a = 0; a < game_live_actor_count; ++a)
    {
        game_actors_transform_outputs[a].position.x += (game_actors_transform_inputs[a].forward_vector.x * game_actors_transform_inputs[a].forward_speed);
        game_actors_transform_outputs[a].position.y += (game_actors_transform_inputs[a].forward_vector.y * game_actors_transform_inputs[a].forward_speed);

        if (game_actors_transform_outputs[a].position.x > 1.f)
        {
            game_actors_transform_outputs[a].position.x = -1.f;
        }

        if (game_actors_transform_outputs[a].position.x < -1.f)
        {
            game_actors_transform_outputs[a].position.x = 1.f;
        }

        if (game_actors_transform_outputs[a].position.y > 1.f)
        {
            game_actors_transform_outputs[a].position.y = -1.f;
        }

        if (game_actors_transform_outputs[a].position.y < -1.f)
        {
            game_actors_transform_outputs[a].position.y = 1.f;
        }

        game_actors_transform_outputs[a].rotation += (game_actors_transform_inputs[a].rotation_speed);
    }

exit:
    return age_result;
}

AGE_RESULT game_apply_player_damping (void)
{
    AGE_RESULT age_result = AGE_SUCCESS;

    game_player_transform_inputs.v.x *= game_player_transform_inputs.damping_factor;
    game_player_transform_inputs.v.y *= game_player_transform_inputs.damping_factor;

    game_player_transform_inputs.u.x *= game_player_transform_inputs.damping_factor;
    game_player_transform_inputs.u.y *= game_player_transform_inputs.damping_factor;

exit:
    return age_result;
}

AGE_RESULT game_process_player_input (void)
{
    AGE_RESULT age_result = AGE_SUCCESS;

    if (is_w_pressed)
    {
        age_result = game_player_increase_speed ();
        if (age_result != AGE_SUCCESS)
        {
            goto exit;
        }
    }

    if (is_up_arrow_pressed)
    {
        age_result = game_player_increase_speed ();
        if (age_result != AGE_SUCCESS)
        {
            goto exit;
        }
    }
    
    if (is_s_pressed)
    {
        age_result = game_player_decrease_speed ();
        if (age_result != AGE_SUCCESS)
        {
            goto exit;
        }
    }

    if (is_down_arrow_pressed)
    {
        age_result = game_player_decrease_speed ();
        if (age_result != AGE_SUCCESS)
        {
            goto exit;
        }
    }

    if (is_d_pressed)
    {
        age_result = game_player_turn_right ();
        if (age_result != AGE_SUCCESS)
        {
            goto exit;
        }
    }

    if (is_right_arrow_pressed)
    {
        age_result = game_player_turn_right ();
        if (age_result != AGE_SUCCESS)
        {
            goto exit;
        }
    }

    if (is_a_pressed)
    {
        age_result = game_player_turn_left ();
        if (age_result != AGE_SUCCESS)
        {
            goto exit;
        }
    }

    if (is_left_arrow_pressed)
    {
        age_result = game_player_turn_left ();
        if (age_result != AGE_SUCCESS)
        {
            goto exit;
        }
    }

    if (is_space_bar_pressed)
    {
        age_result = game_player_shoot_bullet ();
        if (age_result != AGE_SUCCESS)
        {
            goto exit;
        }
    }

exit:
    return age_result;
}

AGE_RESULT game_update (size_t delta_time)
{
    AGE_RESULT age_result = AGE_SUCCESS;

    age_result = game_process_player_input ();
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }
   
    age_result = game_update_player_actor_output_positions ();
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

    age_result = game_apply_player_damping ();
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

    age_result = graphics_update_transforms_buffer_data (&game_player_transform_outputs, game_actors_transform_outputs, game_live_actor_count);
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

    game_delta_time = delta_time;

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

void game_shutdown (void)
{
    graphics_shutdown ();
    vulkan_interface_shutdown ();
    
    utils_free (game_actors_transform_inputs);
    utils_free (game_actors_transform_outputs);
}