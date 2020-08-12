#include "game.h"
#include "vulkan_graphics.h"
#include "types.h"
#include "vulkan_interface.h"

#include "utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>


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
float game_player_shooting_interval_msecs = 100.f;
float game_secs_since_last_shot = 0;

bullet_transform_inputs* game_bullets_transform_inputs = NULL;
size_t* game_bullets_current_lifetimes_msecs = NULL;
actor_transform_outputs* game_bullets_transform_outputs = NULL;
actor_id* game_bullets_ids = NULL;

float game_bullets_max_lifetime_msecs = 500.f;

asteroid_transform_inputs* game_asteroids_transform_inputs = NULL;
actor_transform_outputs* game_asteroids_transform_outputs = NULL;
actor_id* game_asteroids_ids = NULL;

size_t game_asteroid_current_max_count = 0;
size_t game_asteroid_live_count = 0;
size_t game_bullet_current_max_count = 0;
size_t game_bullet_live_count = 0;
const size_t game_ASTEROID_BATCH_SIZE = 20;
const size_t game_BULLET_BATCH_SIZE = 20;

RECT window_rect;
int32_t last_mouse_x;
int32_t last_mouse_y;

size_t game_delta_time = 0;


AGE_RESULT game_reserve_memory_for_asteroids_bullets ()
{
    AGE_RESULT age_result = AGE_SUCCESS;

    game_asteroid_current_max_count += game_ASTEROID_BATCH_SIZE;
    game_asteroids_transform_inputs = (asteroid_transform_inputs*)utils_calloc (game_asteroid_current_max_count, sizeof (asteroid_transform_inputs));
    game_asteroids_transform_outputs = (actor_transform_outputs*)utils_calloc (game_asteroid_current_max_count, sizeof (actor_transform_outputs));
    game_asteroids_ids = (actor_id*)utils_calloc (game_asteroid_current_max_count, sizeof (actor_id));

    game_bullet_current_max_count += game_BULLET_BATCH_SIZE;
    game_bullets_transform_inputs = (bullet_transform_inputs*)utils_calloc (game_bullet_current_max_count, sizeof (bullet_transform_inputs));
    game_bullets_transform_outputs = (actor_transform_outputs*)utils_calloc (game_bullet_current_max_count, sizeof (actor_transform_outputs));
    game_bullets_current_lifetimes_msecs = (size_t*)utils_calloc (game_bullet_current_max_count, sizeof (size_t));
    game_bullets_ids = (actor_id*)utils_calloc (game_bullet_current_max_count, sizeof (actor_id));

exit: // clean up allocations done in this function

    return age_result;
}

AGE_RESULT game_init (const HINSTANCE h_instance, const HWND h_wnd)
{
    AGE_RESULT age_result = AGE_SUCCESS;

    game_player_transform_inputs.time_msecs_to_come_to_rest = 500.f;
    game_player_transform_inputs.forward_vector.x = 0;
    game_player_transform_inputs.forward_vector.y = 1;
    game_player_transform_inputs.acceleration = 0.000125f;
    game_player_transform_inputs.deceleration = -0.0000625f;
    game_player_transform_inputs.rotation_speed = 0.005f;
    game_player_transform_inputs.max_velocity = 0.05f;

    GetClientRect (h_wnd, &window_rect);

    srand (time (NULL));

    age_result = game_reserve_memory_for_asteroids_bullets ();
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

    age_result = vulkan_interface_init (h_instance, h_wnd);
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

    age_result = graphics_init (game_asteroid_current_max_count, game_asteroid_live_count, game_bullet_current_max_count, game_bullet_live_count);
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

exit:  // place to clean up local allocations
    return age_result;
}

AGE_RESULT game_asteroid_add (void)
{
    AGE_RESULT age_result = AGE_SUCCESS;

    if (game_asteroid_live_count == game_asteroid_current_max_count)
    {
        game_asteroid_current_max_count += game_ASTEROID_BATCH_SIZE;

        game_asteroids_transform_inputs = (asteroid_transform_inputs*) utils_realloc (game_asteroids_transform_inputs, sizeof (asteroid_transform_inputs) * game_asteroid_current_max_count);
        game_asteroids_transform_outputs = (actor_transform_outputs*) utils_realloc (game_asteroids_transform_outputs, sizeof (actor_transform_outputs) * game_asteroid_current_max_count);
        game_asteroids_ids = (actor_id*) utils_realloc (game_asteroids_ids, game_asteroid_current_max_count);
        
        age_result = graphics_create_transforms_buffer (game_asteroid_current_max_count + game_bullet_current_max_count);
        if (age_result != AGE_SUCCESS)
        {
            goto exit;
        }
    }

    srand (rand ());

    game_asteroids_transform_outputs[game_asteroid_live_count].position.x = ((float)rand () / (float)RAND_MAX) * 2 - 1;
    game_asteroids_transform_outputs[game_asteroid_live_count].position.y = ((float)rand () / (float)RAND_MAX) * 2 - 1;
    game_asteroids_transform_outputs[game_asteroid_live_count].rotation = (float)rand () / (float)RAND_MAX * 3.14f;

    game_asteroids_transform_inputs[game_asteroid_live_count].forward_vector.x = ((float)rand () / (float)RAND_MAX) * 2 - 1;
    game_asteroids_transform_inputs[game_asteroid_live_count].forward_vector.y = ((float)rand () / (float)RAND_MAX) * 2 - 1;
    game_asteroids_transform_inputs[game_asteroid_live_count].forward_speed = ((float)rand () / (float)RAND_MAX) / 1000.f;
    game_asteroids_transform_inputs[game_asteroid_live_count].rotation_speed = (((float)rand () / (float)RAND_MAX)) / 500.f;

    game_asteroids_ids[game_asteroid_live_count].id = game_asteroid_live_count;

    ++game_asteroid_live_count;

    age_result = graphics_update_command_buffers (game_asteroid_live_count, game_bullet_live_count);
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

exit:
    return age_result;;
}

AGE_RESULT game_process_left_mouse_click (const int32_t x, const int32_t y)
{
    AGE_RESULT age_result = AGE_SUCCESS;

    age_result = game_asteroid_add ();
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

exit:
    return age_result;
}

AGE_RESULT game_asteroid_remove (size_t index_to_remove)
{
    AGE_RESULT age_result = AGE_SUCCESS;

    srand (rand ());
    //size_t actor_index_to_remove = (size_t)(((float)rand () / (float)RAND_MAX) * game_asteroid_live_count);

    if (game_asteroid_live_count > 0)
    {
        for (size_t a = index_to_remove; a < game_asteroid_live_count; ++a)
        {
            game_asteroids_transform_inputs[a] = game_asteroids_transform_inputs[a + 1];
        }

        for (size_t a = index_to_remove; a < game_asteroid_live_count; ++a)
        {
            game_asteroids_transform_outputs[a] = game_asteroids_transform_outputs[a + 1];
        }

        for (size_t a = index_to_remove; a < game_asteroid_live_count; ++a)
        {
            game_asteroids_ids[a] = game_asteroids_ids[a + 1];
        }

        --game_asteroid_live_count;
    }

    age_result = graphics_update_command_buffers (game_asteroid_live_count, game_bullet_live_count);
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

exit:
    return age_result;
}

AGE_RESULT game_process_right_mouse_click (const int32_t x, const int32_t y)
{
    AGE_RESULT age_result = AGE_SUCCESS;

    age_result = game_asteroid_remove ((size_t)(((float)rand () / (float)RAND_MAX) * game_asteroid_live_count));
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

    if (hypot (game_player_transform_inputs.v.x, game_player_transform_inputs.v.y) > game_player_transform_inputs.max_velocity)
    {
        goto exit;
    }

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

    if (hypot (game_player_transform_inputs.v.x, game_player_transform_inputs.v.y) > game_player_transform_inputs.max_velocity)
    {
        goto exit;
    }

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

    game_player_transform_inputs.rotation -= (game_player_transform_inputs.rotation_speed * game_delta_time);
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
    
    game_player_transform_inputs.rotation += (game_player_transform_inputs.rotation_speed * game_delta_time);
    game_player_transform_outputs.rotation = game_player_transform_inputs.rotation;

    age_result = game_update_player_vectors ();
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }
exit:
    return age_result;
}

AGE_RESULT game_bullet_add (void)
{
    AGE_RESULT age_result = AGE_SUCCESS;

    if (game_bullet_live_count == game_bullet_current_max_count)
    {
        game_bullet_current_max_count += game_BULLET_BATCH_SIZE;

        game_bullets_transform_inputs = (bullet_transform_inputs*)utils_realloc (game_bullets_transform_inputs, sizeof (bullet_transform_inputs) * game_bullet_current_max_count);
        game_bullets_transform_outputs = (actor_transform_outputs*)utils_realloc (game_bullets_transform_outputs, sizeof (actor_transform_outputs) * game_bullet_current_max_count);
        game_bullets_current_lifetimes_msecs = (size_t*)utils_realloc (game_bullets_current_lifetimes_msecs, sizeof (size_t) * game_bullet_current_max_count);
        game_bullets_ids = (actor_id*)utils_realloc (game_bullets_ids, sizeof (actor_id) * game_bullet_current_max_count);

        age_result = graphics_create_transforms_buffer (game_asteroid_current_max_count + game_bullet_current_max_count);
        if (age_result != AGE_SUCCESS)
        {
            goto exit;
        }
    }

    srand (rand ());

    game_bullets_transform_outputs[game_bullet_live_count].position.x = game_player_transform_outputs.position.x;
    game_bullets_transform_outputs[game_bullet_live_count].position.y = game_player_transform_outputs.position.y;
    
    game_bullets_transform_outputs[game_bullet_live_count].rotation = game_player_transform_outputs.rotation;
    
    game_bullets_transform_inputs[game_bullet_live_count].forward_vector = game_player_transform_inputs.forward_vector;
    game_bullets_transform_inputs[game_bullet_live_count].speed = (float2_length (&game_player_transform_inputs.v) / (float)game_delta_time) + 0.005f;

    game_bullets_current_lifetimes_msecs[game_bullet_live_count] = 0;
    game_bullets_ids[game_bullet_live_count].id = game_bullet_live_count;

    ++game_bullet_live_count;

    age_result = graphics_update_command_buffers (game_asteroid_live_count, game_bullet_live_count);
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }
exit:
    return age_result;
}

AGE_RESULT game_bullet_remove (size_t index_to_remove)
{
    AGE_RESULT age_result = AGE_SUCCESS;

    if (game_bullet_live_count > 0)
    {
        for (size_t b = index_to_remove; b < game_bullet_live_count; ++b)
        {
            game_bullets_transform_inputs[b] = game_bullets_transform_inputs[b + 1];
        }
        
        for (size_t b = index_to_remove; b < game_bullet_live_count; ++b)
        {
            game_bullets_transform_outputs[b] = game_bullets_transform_outputs[b + 1];
        }

        for (size_t b = index_to_remove; b < game_bullet_live_count; ++b)
        {
            game_bullets_ids[b] = game_bullets_ids[b + 1];
        }

        for (size_t b = index_to_remove; b < game_bullet_live_count; ++b)
        {
            game_bullets_current_lifetimes_msecs[b] = game_bullets_current_lifetimes_msecs[b + 1];
        }
        
        --game_bullet_live_count;
    }

    age_result = graphics_update_command_buffers (game_asteroid_live_count, game_bullet_live_count);
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

exit:
    return age_result;
}

AGE_RESULT game_player_attempt_to_shoot (void)
{
    AGE_RESULT age_result = AGE_SUCCESS;

    if (game_secs_since_last_shot > game_player_shooting_interval_msecs)
    {
        age_result = game_bullet_add ();
        if (age_result != AGE_SUCCESS)
        {
            goto exit;
        }

        game_secs_since_last_shot = 0;
    }

exit:
    return age_result;
}

AGE_RESULT game_bullets_check_life (void)
{
    AGE_RESULT age_result = AGE_SUCCESS;

    for (size_t b = 0; b < game_bullet_live_count; ++b)
    {
        game_bullets_current_lifetimes_msecs[b] += game_delta_time;
    }

    for (size_t b = 0; b < game_bullet_live_count; ++b)
    {
        if (game_bullets_current_lifetimes_msecs[b] > game_bullets_max_lifetime_msecs)
        {
            age_result = game_bullet_remove (b);
            if (age_result != AGE_SUCCESS)
            {
                goto exit;
            }
        }
    }

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

AGE_RESULT game_update_player_asteroids_bullets_output_positions (void)
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

    for (size_t a = 0; a < game_asteroid_live_count; ++a)
    {
        game_asteroids_transform_outputs[a].position.x += (game_asteroids_transform_inputs[a].forward_vector.x * game_asteroids_transform_inputs[a].forward_speed * game_delta_time);
        game_asteroids_transform_outputs[a].position.y += (game_asteroids_transform_inputs[a].forward_vector.y * game_asteroids_transform_inputs[a].forward_speed * game_delta_time);

        if (game_asteroids_transform_outputs[a].position.x > 1.f)
        {
            game_asteroids_transform_outputs[a].position.x = -1.f;
        }

        if (game_asteroids_transform_outputs[a].position.x < -1.f)
        {
            game_asteroids_transform_outputs[a].position.x = 1.f;
        }

        if (game_asteroids_transform_outputs[a].position.y > 1.f)
        {
            game_asteroids_transform_outputs[a].position.y = -1.f;
        }

        if (game_asteroids_transform_outputs[a].position.y < -1.f)
        {
            game_asteroids_transform_outputs[a].position.y = 1.f;
        }

        game_asteroids_transform_outputs[a].rotation += (game_asteroids_transform_inputs[a].rotation_speed) * game_delta_time;
    }

    for (size_t b = 0; b < game_bullet_live_count; ++b)
    {
        game_bullets_transform_outputs[b].position.x += (game_bullets_transform_inputs[b].forward_vector.x * (game_bullets_transform_inputs[b].speed * game_delta_time));
        game_bullets_transform_outputs[b].position.y += (game_bullets_transform_inputs[b].forward_vector.y * (game_bullets_transform_inputs[b].speed * game_delta_time));

        if (game_bullets_transform_outputs[b].position.x > 1.f)
        {
            age_result = game_bullet_remove (b);
            if (age_result != AGE_SUCCESS)
            {
                goto exit;
            }
        }

        if (game_bullets_transform_outputs[b].position.x < -1.f)
        {
            age_result = game_bullet_remove (b);
            if (age_result != AGE_SUCCESS)
            {
                goto exit;
            }
        }

        if (game_bullets_transform_outputs[b].position.y > 1.f)
        {
            age_result = game_bullet_remove (b);
            if (age_result != AGE_SUCCESS)
            {
                goto exit;
            }
        }

        if (game_bullets_transform_outputs[b].position.y < -1.f)
        {
            age_result = game_bullet_remove (b);
            if (age_result != AGE_SUCCESS)
            {
                goto exit;
            }
        }
    }

exit:
    return age_result;
}

AGE_RESULT game_player_apply_damping (void)
{
    AGE_RESULT age_result = AGE_SUCCESS;

    float damping_value = 1;

    if (game_delta_time < game_player_transform_inputs.time_msecs_to_come_to_rest)
    {
        damping_value = (1 - (game_delta_time / game_player_transform_inputs.time_msecs_to_come_to_rest));
    }

    game_player_transform_inputs.v.x *= damping_value;
    game_player_transform_inputs.v.y *= damping_value;

    game_player_transform_inputs.u.x *= damping_value;
    game_player_transform_inputs.u.y *= damping_value;

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
        age_result = game_player_attempt_to_shoot ();
        if (age_result != AGE_SUCCESS)
        {
            goto exit;
        }
    }

exit:
    return age_result;
}

AGE_RESULT game_collision_checks (void)
{
    AGE_RESULT age_result = AGE_SUCCESS;

    for (size_t b = 0; b < game_bullet_live_count; ++b)
    {
        for (size_t a = 0; a < game_asteroid_live_count; ++a)
        {
            float2 diff = {
                game_asteroids_transform_outputs[a].position.x - game_bullets_transform_outputs[b].position.x,
                game_asteroids_transform_outputs[a].position.y - game_bullets_transform_outputs[b].position.y,
            };

            if (hypotf (diff.x, diff.y) < 0.1f)
            {
                age_result = game_bullet_remove (b);
                if (age_result != AGE_SUCCESS)
                {
                    goto exit;
                }

                age_result = game_asteroid_remove (a);
                if (age_result != AGE_SUCCESS)
                {
                    goto exit;
                }                
            }
        }
    }

exit:
    return age_result;
}

AGE_RESULT game_update (size_t delta_msecs)
{
    AGE_RESULT age_result = AGE_SUCCESS;

    game_delta_time = delta_msecs;

    game_secs_since_last_shot += game_delta_time;;

    age_result = game_process_player_input ();
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

    age_result = game_update_player_asteroids_bullets_output_positions ();
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

    age_result = game_player_apply_damping ();
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

    age_result = game_bullets_check_life ();
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

    age_result = game_collision_checks ();
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

    age_result = graphics_update_transforms_buffer_data (
        &game_player_transform_outputs, 
        game_asteroids_transform_outputs, 
        game_asteroid_live_count,
        game_asteroid_current_max_count,
        game_bullets_transform_outputs,
        game_bullet_live_count,
        game_bullet_current_max_count
    );
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

void game_shutdown (void)
{
    graphics_shutdown ();
    vulkan_interface_shutdown ();
    
    utils_free (game_asteroids_transform_inputs);
    utils_free (game_asteroids_transform_outputs);
    utils_free (game_asteroids_ids);

    utils_free (game_bullets_transform_inputs);
    utils_free (game_bullets_transform_outputs);
    utils_free (game_bullets_current_lifetimes_msecs);
    utils_free (game_bullets_ids);
}