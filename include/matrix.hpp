#pragma once
// Stl
#include <vector>
#include <array>
#include <iostream>

template<typename T, size_t M, size_t N>
class Matrix {
public:
    Matrix() : _rows(M), _cols(N), _data(M * N, T{}) {
        if constexpr (M == N) {
            setEye();
        }
    }

    Matrix(const std::initializer_list<T>& list) : _rows(M), _cols(N) {
        if (list.size() != M * N) {
            throw std::invalid_argument("Initializer list size does not match matrix dimensions.");
        }
        std::copy(std::begin(list), std::end(list), std::begin(_data));
    }
    
    Matrix(const Matrix& other) = default;
    Matrix& operator=(const Matrix& other) = default;
    Matrix(Matrix&& other) noexcept = default;
    Matrix& operator=(Matrix&& other) noexcept = default;

    T operator()(size_t row, size_t col) const { return _data[row * _cols + col]; }
    T& operator()(size_t row, size_t col) { return _data[row * _cols + col]; }
    size_t size() const { return _rows * _cols; }
    T cols() const { return _cols; }
    T rows() const { return _rows; }

    Matrix<T, M, N> operator+(const Matrix<T, M, N>& other) const {
        Matrix result;
        std::transform(_data.begin(), _data.end(), other._data.begin(), result._data.begin(),
                       std::plus<>());
        return result;
    }

    Matrix<T, M, N> operator-(const Matrix<T, M, N>& other) const {
        Matrix<T, M, N> result;
        std::transform(_data.begin(), _data.end(), other._data.begin(), result._data.begin(),
                       std::minus<>());
        return result;
    }

    Matrix<T, M, N> operator*(const Matrix<T, M, N>& other) const {
        Matrix<T, M, N> result;
        for (size_t i = 0; i < _rows; i++) {
            for (size_t j = 0; j < _cols; j++) {
                result(i, j) = T{};
                for (size_t k = 0; k < _cols; k++) {
                    result(i, j) += (*this)(i, k) * other(k, j);
                }
            }
        }
        return result;
    }

    friend std::ostream& operator<<(std::ostream& os, const Matrix& m) {
        for (size_t i = 0; i < m._rows; i++) {
            os << "[ ";
            for (size_t j = 0; j < m._cols; j++) {
                os << m(i, j);
                if (j + 1 < m._cols)
                    os << ", ";
            }
            os << " ]\n";
        }
        return os;
    }

    void setEye() {
        std::fill(std::begin(_data), std::end(_data), 0);
        for (size_t i = 0; i < std::min(_rows, _cols); i++) {
            _data[i * _cols + i] = T{1};
        }
    }

    void setScale(const T& sx, const T& sy, const T& sz) {
        if constexpr (M >= 4 && N >= 4) {
            Matrix<T, 4, 4> S;
            S(0, 0) = sx;
            S(1, 1) = sy;
            S(2, 2) = sz;
            *this = S * *this;  // order of multiplication matters
        } else {
            throw std::invalid_argument("Matrix must be at least 4x4 to set scale.");
        }
    }
    void setTranslation(const T& tx, const T& ty, const T& tz) {
        if constexpr (M >= 4 && N >= 4) {
            Matrix<T, 4, 4> T;
            T(0, 3) = tx;
            T(1, 3) = ty;
            T(2, 3) = tz;
            *this = T * *this;  // order of multiplication matters
        } else {
            throw std::invalid_argument("Matrix must be at least 4x4 to set translation.");
        }
    }
    void setRotation(const T& alpha, const T& beta, const T& gamma) {
        if constexpr (M >= 4 && N >= 4) {
            Matrix<T, 4, 4> R = getRotationMatrix(alpha, beta, gamma);
            *this = *this * R;  // order of multiplication matters
        } else {
            throw std::invalid_argument("Matrix must be at least 4x4 to set rotation.");
        }
    }

    Matrix<T, M, N> getRotationMatrix(T alpha, T beta, T gamma) const {
        // totation matrices follows the right-hand rule (counter-clockwise rotation)
        auto cos_alpha = cos(alpha);
        auto sin_alpha = sin(alpha);
        auto cos_beta = cos(beta);
        auto sin_beta = sin(beta);
        auto cos_gamma = cos(gamma);
        auto sin_gamma = sin(gamma);
        Matrix<T, 4, 4> Rx{1, 0, 0, 0, 0, cos_alpha, -sin_alpha, 0, 0, sin_alpha, cos_alpha, 0, 0, 0, 0, 1};
        Matrix<T, 4, 4> Ry{cos_beta, 0, sin_beta, 0, 0, 1, 0, 0, -sin_beta, 0, cos_beta, 0, 0, 0, 0, 1};
        Matrix<T, 4, 4> Rz{cos_gamma, -sin_gamma, 0, 0, sin_gamma, cos_gamma, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
        return Rz * Ry * Rx;  // order of multiplication Doesn't matter
    }

private:
    template <typename T, size_t M, size_t K, size_t N>
    friend Matrix<T, M, N> operator*(const Matrix<T, M, K>& a, const Matrix<T, K, N>& b);
    size_t _rows;
    size_t _cols;
    std::array<T, M * N> _data;
};

template <typename T, size_t M, size_t K, size_t N>
Matrix<T, M, N> operator*(const Matrix<T, M, K>& a, const Matrix<T, K, N>& b) {
    Matrix<T, M, N> result;
    for (size_t i = 0; i < M; i++) {
        for (size_t j = 0; j < N; j++) {
            result(i, j) = T{};
            for (size_t k = 0; k < K; k++) {
                result(i, j) += a(i, k) * b(k, j);
            }
        }
    }
    return result;
}