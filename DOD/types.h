#pragma once


typedef struct float2_
{
    float x;
    float y;
} float2;

typedef struct float3_
{
    float x;
    float y;
    float z;
} float3;

typedef struct player_transform_inputs_
{
    float acceleration;
    float2 v;
    float2 u;
    float2 forward_vector;
    float rotation;
    float rotation_speed;
    float damping_factor;
    float firing_rate;
} player_transform_inputs;

typedef struct bullet_transform_inputs_
{
    float2 forward_vector;
    float speed;
} bullet_transform_inputs;

typedef struct actor_transform_inputs_
{
    float2 forward_vector;
    float forward_speed;
    float rotation;
    float rotation_speed;
} actor_transform_inputs;

typedef struct actor_transform_outputs_
{
    float2 position;
    float rotation;
} actor_transform_outputs;


float float2_length (const float2* in_vector);
void float2_normalize (float2* in_vector);
