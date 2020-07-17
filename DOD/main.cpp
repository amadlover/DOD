#include <iostream>
#include <vector>

struct transforms
{
	std::vector<float> positions;
	std::vector<float> rotations;
	std::vector<float> scales;
};

struct asset_offsets
{
	std::vector<size_t> positions_offsets;
	std::vector<size_t> rotations_offsets;
	std::vector<size_t> scales_offsets;
};

struct assets
{
	std::vector<std::string> names;
};

int main ()
{
	std::cout << "Hello DOD\n";
}
