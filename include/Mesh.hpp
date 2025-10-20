#pragma once
// stl
#include <array>
#include <vector>
// inernal
#include "vector.hpp"
#include "matrix.hpp"

constexpr int MAX_NUM_POLY_VERTICES = 10;
constexpr int MAX_NUM_TRIANGLES_AFTER_CLIPPING = 10;

// Facse and triangle are the same and could be used interchangeably

// face stores triangle vertices indices
struct Face {
    vec3f_t normal;
    vec2f_t a_uv;  // texture coordinates
    vec2f_t b_uv;
    vec2f_t c_uv;
    int a;
    int b;
    int c;
    uint32_t color;
};

// Stores the actual vertex values x and y
struct Triangle {
    std::array<Matrix<float, 4, 1>, 3> points;
    std::array<vec2f_t, 3> text_coords;  // texture coordinates
    vec3f_t normal;
    uint32_t color;
};

struct Polygon {
    std::array<vec3f_t, MAX_NUM_POLY_VERTICES> vertices;
    int num_of_vertices;
};

struct Mesh {
    std::vector<vec3f_t> vertices;  // vector the mesh vertices
    std::vector<Face> faces;  // each face stores the indices of the vertices that make up the face
    vec3f_t rotation{0, 0, 0};     //roation with x, y, z
    vec3f_t scale{1.0, 1.0, 1.0};        // scale with x, y, z
    vec3f_t translation{0, 0, 0};  // translation with x, y, z
};
