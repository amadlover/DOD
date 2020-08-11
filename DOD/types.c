#include "types.h"

#include <math.h>
#include <stdio.h>

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