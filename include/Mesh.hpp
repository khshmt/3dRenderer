#pragma once

#include <array>
#include <vector>
#include "Vector.hpp"

// Facse and trianglke are the same word and could be used interchangeably

// face stores triangle vertices indices
struct Face {
    int a;
    int b;
    int c;
    uint32_t color;
};

//stores the actual vertex values x and y
struct Triangle {
    std::array<math::Vector<float, 2>, 3> points;
};

struct Mesh {
    std::vector<Face> faces;
    std::vector<math::Vector<float, 3>> vertices;
    math::Vector<float,3> rotation;
};