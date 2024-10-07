#pragma once

#include <array>
#include "Vector.hpp"

// face stores vertex index
struct Face {
    int a;
    int b;
    int c;
};

//stores the actual vertex values x and y
class Triangle {
   public:
    std::array<math::Vector<float, 2>, 3> points;
};