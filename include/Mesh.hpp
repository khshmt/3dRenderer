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
    using point = vec2f_t;

    std::array<point, 3> points;
    float avg_depth;
    uint32_t color;
};

struct Mesh {
    using vertex = vec3f_t;

    std::vector<Face> faces;
    std::vector<vertex> vertices;
    vec3f_t rotation;
};