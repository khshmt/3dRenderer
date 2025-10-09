#pragma once
// stl
#include <array>
#include <vector>
// inernal
#include "vector.hpp"

using point = vec2f_t;
using vertex = vec3f_t;

// Facse and triangle are the same and could be used interchangeably

// face stores triangle vertices indices
struct Face {
    vec3f_t normal;
    int a;
    int b;
    int c;
    uint32_t color;
};

//stores the actual vertex values x and y
struct Triangle {
    std::array<point, 3> points;
    vec3f_t normal;
    float avg_depth;
    uint32_t color;
};

struct Mesh {
    std::vector<vertex> vertices;  // vector the mesh vertices
    std::vector<Face> faces;  // each face stores the indices of the vertices that make up the face
    vec3f_t rotation{0, 0, 0};     //roation with x, y, z
    vec3f_t scale{1.0, 1.0, 1.0};        // scale with x, y, z
    vec3f_t translation{0, 0, 0};  // translation with x, y, z
};
