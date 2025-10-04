#pragma once
#include <cmath>
#include <cstdlib>
#include <stdexcept>
#include <tuple>

namespace {
template <typename T, size_t N>
struct vector {
public:
    constexpr vector() noexcept : _x(0), _y(0), _z(0) {}
    vector(T x, T y, T z = 0) : _x(x), _y(y), _z(z) { static_assert(N == 2 || N == 3); }

    T& x() { return _x; }
    T& y() { return _y; }
    T& z() {
        if (N == 2) {
            throw std::invalid_argument("2D vectors have no z component.");
        }
        return _z;
    }

    T x() const { return _x; }
    T y() const { return _y; }
    T z() const {
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

    std::tuple<T, T, T> get() {
        if (N == 3)
            return {x(), y(), z()};
        if (N == 2)
            return {x(), y(), 0.f};
    }

    float magnitude() {
        if (N == 2)
            return std::sqrt(_x * _x + _y * _y);
        else
            return std::sqrt(_x * _x + _y * _y + _z * _z);
    }

    vector<T, N> Add(const vector<T, N>& otherVec) {
        _x = _x + otherVec.x();
        _y = _y + otherVec.y();
        if (N == 3)
            _z = _z + otherVec.z();
    }

    vector<T, N> operator+(vector<T, N>& other) {
        if (N == 2)
            return {(_x + other.x()), (_y + other.y())};
        if (N == 3)
            return {(_x + other.x()), (_y + other.y()), (_z + other.z())};
    }

    void Subtract(vector<T, N>& otherVec) {
        _x = _x - otherVec.x();
        _y = _y - otherVec.y();
        if (N == 3)
            _z = _z - otherVec.z();
    }

    vector<T, N> operator-(vector<T, N>& other) {
        if (N == 2)
            return {(_x - other.x()), (_y - other.y())};
        if (N == 3)
            return {(_x - other.x()), (_y - other.y()), (_z - other.z())};
    }

    void scale(float factor) {
        _x *= factor;
        _y *= factor;
        if (N == 3)
            _z *= factor;
    }

    void divide(float factor) {
        _x /= factor;
        _y /= factor;
        if (N == 3)
            _z /= factor;
    }

    // cross product
    vector<T, 3> operator^(vector<T, 3>& other) {
        return {(_y * other.z() - _z * other.y()), (_z * other.x() - _x * other.z()),
                (_x * other.y() - _y * other.x())};
    }

    // dot product
    float operator*(vector<T, N>& other) {
        if (N == 2)
            return _x * other.x() + _y * other.y();
        if (N == 3)
            return _x * other.x() + _y * other.y() + _z * other.z();
    }

    void normalize() {
        auto mag = this->magnitude();
        _x /= mag;
        _y /= mag;
        if (N == 3)
            _z /= mag;
    }

private:
    T _x{0};
    T _y{0};
    T _z{0};
};

using vec2f_t = vector<float, 2>;
using vec3f_t = vector<float, 3>;

using vec2i_t = vector<int, 2>;
using vec3i_t = vector<int, 3>;

using vec2d_t = vector<double, 2>;
using vec3d_t = vector<double, 3>;

}  // namespace