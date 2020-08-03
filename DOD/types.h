#pragma once


typedef struct vec2_
{
    float x;
    float y;
} vec2;

typedef struct vec3_
{
    float x;
    float y;
    float z;
} vec3;

typedef struct actor_transform_inputs_
{
    vec2 forward_vector;
    float forward_speed;
    float rotation;
    float rotation_speed;
    float damping;
} actor_transform_inputs;

typedef struct actor_transform_outputs_
{
    vec2 position;
    float rotation;
} actor_transform_outputs;

void vec2_normalize (vec2* in_vector);