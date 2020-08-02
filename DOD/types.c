#include "types.h"

#include <math.h>

void vec2_normalize (vec2* in_vector)
{
    float length = (float)sqrt ((in_vector->x * in_vector->x) + (in_vector->y * in_vector->y));

    in_vector->x /= length;
    in_vector->y /= length;
}