#pragma once
// STL
#include <atomic>
#include <memory>
#include <thread>
#include <vector>
// Internal
#include "Mesh.hpp"
// 3d-Party_Libs
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

class Renderer {
   public:
    bool initializeWindow(bool fullscreen = false);
    void setupWindow(const std::string& obj_file_path);
    void destroyWindow();
    void process_input();
    bool getWindowState();
    void update();
    void render();

   private:
    void drawGrid();
    void drawPixel(int x, int y, uint32_t color);
    void drawRect(int x, int y, int width, int height, uint32_t color);
    void drawLine(int x0, int y0, int x1, int y1, uint32_t color);
    void drawTriangle(Triangle& tri, uint32_t color);
    void rasterizeTriangle(Triangle& tri, uint32_t color);
    void rasterizeFlatBottomTriangle(int x0 , int y0, int x1, int y1, int x2, int y2, uint32_t color = 0xFFFF0000);
    void rasterizeFlatTopTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color = 0xFFFF0000);
    void renderColorBuffer();
    void clearColorBuffer(uint32_t color);
    void loadObjFileData(const std::string& obj_file_path);
    math::Vector<float, 2> project(math::Vector<float, 3>& point);

   private:
    std::atomic_bool is_running = false;
    std::atomic_bool pause{false};
    std::atomic_bool _enableFaceCulling{false};

    std::atomic_bool _wireframeModel{true};
    std::atomic_bool _VerticesModel{false};
    std::atomic_bool _raterizeModel{false};

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

    std::vector<uint32_t> color_buffer;
    std::vector<Triangle> triangles_to_render;
    std::vector<Triangle> last_triangles_to_render;
    Mesh mesh;

    math::Vector<float, 3> camera_position = {0, 0, 0};
    math::Vector<float, 3> rotation = {0.0, 0.0, 0.0};

};