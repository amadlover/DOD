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
    float2 v;
    float2 u;
    float2 forward_vector;
    float rotation;
    float damping;
} player_transform_inputs;

typedef struct actor_transform_inputs_
{
    float2 forward_vector;
    float forward_speed;
    float rotation;
    float rotation_speed;
    float damping;
} actor_transform_inputs;

typedef struct actor_transform_outputs_
{
    float2 position;
    float rotation;
} actor_transform_outputs;

void vec2_normalize (float2* in_vector);