#pragma once
#include <Eigen/Dense>
#include <concepts>
#include <iostream>

namespace Eigen {
template <int D, typename Derived>
inline auto& get(Eigen::DenseCoeffsBase<Derived, WriteAccessors>& p) {
    return p.coeffRef(D);
}

template <int D, typename Derived>
inline const auto get(const Eigen::DenseCoeffsBase<Derived, ReadOnlyAccessors>& p) {
    return p.coeff(D);
}

template <typename Derived>
concept StaticSizePlainObjectBase =
    std::derived_from<Derived, Eigen::PlainObjectBase<Derived>> &&
    (Derived::RowsAtCompileTime > 0 && Derived::ColsAtCompileTime > 0);

template <typename Derived>
concept StaticSizeEigenBase = std::derived_from<Derived, Eigen::EigenBase<Derived>> &&
                              (Derived::RowsAtCompileTime > 0 && Derived::ColsAtCompileTime > 0);
}  // namespace Eigen

namespace std {

template <typename Derived>
requires Eigen::StaticSizeEigenBase<Derived> struct tuple_size<Derived> {
    constexpr static int value = Derived::RowsAtCompileTime * Derived::ColsAtCompileTime;
};
template <size_t D, typename Derived>
    requires Eigen::StaticSizeEigenBase<Derived> &&
    (D < Derived::RowsAtCompileTime * Derived::ColsAtCompileTime) struct tuple_element<D, Derived> {
    using type = typename Derived::Scalar;
};
}  // namespace std


// get rotation matrix follows the right-hand rule (counter-clockwise rotation)
Eigen::Matrix3f getRotationMatrix(float alpha, float beta, float gamma) {
    float cos_alpha = std::cos(alpha);
    float sin_alpha = std::sin(alpha);
    float cos_beta = std::cos(beta);
    float sin_beta = std::sin(beta);
    float cos_gamma = std::cos(gamma);
    float sin_gamma = std::sin(gamma);

    Eigen::Matrix3f Rx;
    Rx << 1, 0, 0, 0, cos_alpha, -sin_alpha, 0, sin_alpha, cos_alpha;

    Eigen::Matrix3f Ry;
    Ry << cos_beta, 0, sin_beta, 0, 1, 0, -sin_beta, 0, cos_beta;

    Eigen::Matrix3f Rz;
    Rz << cos_gamma, -sin_gamma, 0, sin_gamma, cos_gamma, 0, 0, 0, 1;

    // Note: rotation order matters — this is Z * Y * X (yaw-pitch-roll)
    return Rz * Ry * Rx;
}