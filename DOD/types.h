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
    vec2 position;
    vec2 direction;
    float speed;
} position_inputs;
