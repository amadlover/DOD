#include <vector>

#define NUM_OBJECTS 10

struct vec3
{
    float x;
    float y;
    float z;
};

struct vec4
{
    float x;
    float y;
    float z;
    float w;
};

struct transforms
{
    transforms (size_t num)
    {
        positions.resize (num);
        rotations.resize (num);
        scales.resize (num);
    }
    std::vector<vec3> positions;
    std::vector<vec4> rotations;
    std::vector<vec3> scales;
};

struct offsets
{
    offsets (size_t num)
    {
        positions_offsets.resize (num);
        rotations_offsets.resize (num);
        scales_offsets.resize (num);
    }
    std::vector<size_t> positions_offsets;
    std::vector<size_t> rotations_offsets;
    std::vector<size_t> scales_offsets;
};


int main ()
{
    transforms input_trs(NUM_OBJECTS);
    transforms output_trs (NUM_OBJECTS);
    offsets offs(NUM_OBJECTS);

    return 0;
}
