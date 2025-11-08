#pragma once
// stl
#include <array>
#include <vector>
// inernal
#include <Eigen/Dense>

constexpr int MAX_NUM_POLY_VERTICES = 10;
constexpr int MAX_NUM_TRIANGLES_AFTER_CLIPPING = 10;

// Facse and triangle are the same and could be used interchangeably

// face stores triangle vertices indices
struct Face {
    Eigen::Vector3f normal;
    Eigen::Vector2f a_uv;  // texture coordinates
    Eigen::Vector2f b_uv;
    Eigen::Vector2f c_uv;
    int a;
    int b;
    int c;
    uint32_t color;
};

// Stores the actual vertex values x and y
struct Triangle {
    std::array<Eigen::Vector4f, 3> points;
    std::array<Eigen::Vector2f, 3> text_coords;  // texture coordinates
    Eigen::Vector3f normal;
    uint32_t color;
};

struct Polygon {
    std::array<Eigen::Vector3f, MAX_NUM_POLY_VERTICES> vertices;
    std::array<Eigen::Vector2f, MAX_NUM_POLY_VERTICES> textcoords;
    int num_of_vertices;
};

struct Mesh {
    std::vector<Eigen::Vector3f> vertices;  // vector the mesh vertices
    std::vector<Face> faces;  // each face stores the indices of the vertices that make up the face
    Eigen::Vector3f rotation{0, 0, 0};     // roation with x, y, z
    Eigen::Vector3f scale{1.0, 1.0, 1.0};        // scale with x, y, z
    Eigen::Vector3f translation{0, 0, 0};  // translation with x, y, z
};
