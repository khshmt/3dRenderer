#pragma once
// STL
#include <array>
#include <atomic>
#include <memory>
#include <thread>
#include <vector>
// Internal
#include "triangle.hpp"
// 3d-Party_Libs
#include <SDL2/SDL.h>

#define N_POINTS (9 * 9 * 9)
#define N_MESH_VERTICES 8
#define N_MESH_FACES (6 * 2)

class Renderer {
   public:
    bool initializeWindow(bool fullscreen = false);
    void setupWindow();
    void destroyWindow();
    void process_input();
    bool getWindowState();
    void update();
    void render();

   private:
    void drawGrid();
    void drawPixel(int x, int y, uint32_t color);
    void drawRect(int x, int y, int width, int height, uint32_t color);
    void drawLine(float x0, float y0, float x1, float y1, uint32_t color);
    void renderColorBuffer();
    void clearColorBuffer(uint32_t color);
    math::Vector<float, 2> project(math::Vector<float, 3> point);

   private:
    std::atomic_bool is_running = false;
    std::atomic_bool pause{false};
    std::atomic_bool left{false};
    std::atomic_bool up{false};

    bool firstFrame{true};

    int _width{800};
    int _height{600};
    float fov_factor = 640;  // for perspective projection
    // float fov_factor = 128; // for isometric projection

    uint32_t previousFrameTime{0};
    const uint32_t fps{30};
    const uint32_t frameTargetTime{1000 / fps};  //the time of one frame

    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window_ptr =
        std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>(nullptr, SDL_DestroyWindow);
    std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> renderer_ptr =
        std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)>(nullptr, SDL_DestroyRenderer);
    std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> color_buffer_texture_ptr =
        std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>(nullptr, SDL_DestroyTexture);

    std::thread th;

    math::Vector<float, 3> camera_position = {0, 0, -5};
    math::Vector<float, 3> cube_rotation = {0.0, 0.0, 0.0};

    std::array<Triangle, N_MESH_FACES> triangles_to_render;
    std::array<math::Vector<float, 3>, N_MESH_VERTICES> mesh_vertices = {{
        {-1.f, -1.f, -1.f},  // 1
        {-1.f, 1.f, -1.f},   // 2
        {1.f, 1.f, -1.f},    // 3
        {1.f, -1.f, -1.f},   // 4
        {1.f, 1.f, 1.f},     // 5
        {1.f, -1.f, 1.f},    // 6
        {-1.f, 1.f, 1.f},    // 7
        {-1.f, -1.f, 1.f}    // 8
    }};
    // this conatiner values are indecies
    std::array<Face, N_MESH_FACES> mesh_faces = {
        {            // front
         {1, 2, 3},  // triangle 1 of front face - clockwise order
         {1, 3, 4},  // triangle 2 of front face
         // right
         {4, 3, 5},
         {4, 5, 6},
         // back
         {6, 5, 7},
         {6, 7, 8},
         // left
         {8, 7, 2},
         {8, 2, 1},
         // top
         {2, 7, 5},
         {2, 5, 3},
         // bottom
         {6, 8, 1},
         {6, 1, 4}}};

    std::vector<uint32_t> color_buffer;
};