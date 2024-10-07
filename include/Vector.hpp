#pragma once
#include <math.h>
#include <cstdlib>
#include <stdexcept>

namespace math {
template <typename T, size_t N>
struct Vector {
   public:
    Vector() : _x(0), _y(0), _z(0) {}
    Vector(T x, T y) : _x(x), _y(y) { static_assert(N == 2); }
    Vector(T x, T y, T z) : _x(x), _y(y), _z(z) { static_assert(N == 3); }

    T& x() { return _x; }
    T& y() { return _y; }
    T& z() {
        if (N == 2) {
            throw std::invalid_argument("2D vectors have no z component.");
        }
        return _z;
    }

    void rotateAroundX(T angle) {
        // static_assert(N == 3);
        //_x is the same
        auto y = _y;
        auto z = _z;
        _y = y * cos(angle) - z * sin(angle);
        _z = y * sin(angle) + z * cos(angle);
    }

    void rotateAroundY(T angle) {
        // static_assert(N == 3);
        auto x = _x;
        auto z = _z;
        _x = x * cos(angle) - z * sin(angle);
        //_y is the same
        _z = x * sin(angle) + z * cos(angle);
    }

    void rotateAroundZ(T angle) {
        auto x = _x;
        auto y = _y;
        _x = x * cos(angle) - y * sin(angle);
        _y = x * sin(angle) + y * cos(angle);
        //_z component is the same
    }

   private:
    T _x{0};
    T _y{0};
    T _z{0};
};
}  // namespace math