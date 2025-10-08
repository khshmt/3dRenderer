#pragma once
// Stl
#include <vector>
#include <iostream>

template<typename T, size_t M, size_t N>
class Matrix {
public:
    Matrix()
        : _rows(M),
          _cols(N), _data(M * N, T{}) { setEye(); }

    Matrix(const std::initializer_list<T>& list) : _rows(M), _cols(N), _data(list) {
        if (list.size() != M * N) {
            throw std::invalid_argument("Initializer list size does not match matrix dimensions.");
        }
    }
    
    Matrix(const Matrix<T, M, N>& other)
        : _rows(other._rows), _cols(other._cols), _data(other._data) {}

    Matrix<T, M, N>& operator=(const Matrix<T, M, N>& other) {
        if (this->_data != other._data) {
            this->_rows = other._rows;
            this->_cols = other._cols;
            this->_data = other._data;
        }
        return *this;
    }

    Matrix(Matrix<T, M, N>&& other) noexcept
        : _rows(other._rows), _cols(other._cols), _data(std::move(other._data)) {
        other._rows = 0;
        other._cols = 0;
    }

    Matrix<T, M, N>& operator=(Matrix<T, M, N>&& other) noexcept {
        if (this != &other) {
            this->_rows = other._rows;
            this->_cols = other._cols;
            this->_data = std::move(other._data);
            other._rows = 0;
            other._cols = 0;
        }
        return *this;
    }

    T operator()(size_t row, size_t col) const { return _data[row * _cols + col]; }
    T& operator()(size_t row, size_t col) { return _data[row * _cols + col]; }

    Matrix<T, M, N> operator+(const Matrix<T, M, N>& other) const {
        checkSameSize(other);
        Matrix<T, M, N> result;
        for (size_t i = 0; i < _data.size(); ++i) {
            result._data[i] = _data[i] + other._data[i];
        }
        return result;
    }

    Matrix<T, M, N> operator-(const Matrix<T, M, N>& other) const {
        checkSameSize(other);
        Matrix<T, M, N> result;
        for (size_t i = 0; i < _data.size(); ++i) {
            result._data[i] = _data[i] - other._data[i];
        }
        return result;
    }

    Matrix<T, M, N> operator*(const Matrix<T, M, N>& other) const {
        checkGoodForMultiplication(other);
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

    // vector OPeration
    void scale(T factor) { 
        if (_cols == 1) {
            for (size_t i = 0; i < _rows; i++) {
                _data[i] *= factor;
            }
        } else {
            throw std::invalid_argument("Matrix is not a vector.");
        }
    }

    void divide(T factor) {
        if (_cols == 1) {
            for (size_t i = 0; i < _rows; i++) {
                _data[i] /= factor;
            }
        } else {
            throw std::invalid_argument("Matrix is not a vector.");
        }
    }

    // cross product
    Matrix<T, M, N> operator^(Matrix<T, M, N>& other) {
        if (_cols == 1 && other.size() == _rows) {
            return {(_data[1] * other._data[2] - _data[2] * other._data[1]),
                    (_data[2] * other._data[0] - _data[0] * other._data[2]),
                    (_data[0] * other._data[1] - _data[1] * other._data[0])};
        } else {
            throw std::invalid_argument("Matrix is not a vector.");
        }
    }

    size_t size() const { return _rows * _cols; }

private:
    T checkGoodForMultiplication(const Matrix<T, M, N>& other) const {
        if (_rows != other._rows || _cols != other._cols) {
            throw std::invalid_argument("Matrix dimensions do not match for multiplication.");
        }
        return T{};
    }

    T checkSameSize(const Matrix<T, M, N>& other) const {
        if (_rows != other._rows || _cols != other._cols) {
            throw std::invalid_argument("Matrix dimensions do not match for addition/subtraction.");
        }
        return T{};
    }

private:
    template <typename T, size_t M, size_t K, size_t N>
    friend Matrix<T, M, N> operator*(const Matrix<T, M, K>& a, const Matrix<T, K, N>& b);
    size_t _rows;
    size_t _cols;
    std::vector<T> _data;
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


//using vec2f_t = Matrix<float, 2, 1>;
//using vec3f_t = Matrix<float, 3, 1>;
//using vec4f_t = Matrix<float, 4, 1>;
//              
//using vec2i_t = Matrix<int, 2, 1>;
//using vec3i_t = Matrix<int, 3, 1>;
//using vec4i_t = Matrix<int, 4, 1>;