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

typedef struct position_inputs_
{
    vec2 direction;
    float speed;
} position_inputs;

typedef struct rotation_inputs_
{
    float speed;
} rotation_inputs;

typedef struct actor_transform_inputs_
{
    vec2 direction;
    float position_speed;
    float rotation_speed;
} actor_transform_inputs;

typedef struct actor_transform_outputs_
{
    vec2 position;
    float rotation;
} actor_transform_outputs;