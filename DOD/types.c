#include "types.h"

#include <math.h>
#include <stdio.h>

float float_max (const float a, const float b)
{
    if (a > b)
    {
        return a;
    }
    else if (b > a)
    {
        return b;
    }
    else
    {
        return -1;
    }
}

float fload_min (const float a, const float b)
{
    if (a < b)
    {
        return a;
    }
    else if (b < a)
    {
        return b;
    }
    else
    {
        return -1;
    }
}

float float_clamp (const float value, const float min, const float max)
{
    if (value > max)
    {
        return max;
    }
    else if (value < min)
    {
        return min;
    }
    else 
    {
        return value;
    }
}

float float2_length (const float2* in_vector)
{
    return hypotf (in_vector->x, in_vector->y);
}

void float2_normalize (float2* in_vector)
{
    float length = hypotf (in_vector->x, in_vector->y);

    in_vector->x /= length;
    in_vector->y /= length;
}