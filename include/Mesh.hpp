#pragma once
// STL
#include <array>
#include <vector>
// INTERNAL
#include "vector.hpp"

// Facse and triangle are the same and could be used interchangeably

// face stores triangle vertices indices
struct Face {
    int a;
    int b;
    int c;
    uint32_t color;
};

//stores the actual vertex values x and y
struct Triangle {
    using point = ::vector<float, 2>;

    std::array<point, 3> points;
};

struct Mesh {
    using vertex = ::vector<float, 3>;

    std::vector<Face> faces;
    std::vector<vertex> vertices;
    ::vector<float, 3> rotation;
};