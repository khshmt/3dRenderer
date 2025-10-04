#pragma once
// STL
#include <atomic>
#include <memory>
#include <thread>
#include <vector>
// Internal
#include "Mesh.hpp"
#include "timer.hpp"
// 3d-Party_Libs
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>


#ifdef _WIN32
#ifdef RENDERER_EXPORTS
#define RENDERER_API __declspec(dllexport)
#else
#define RENDERER_API __declspec(dllimport)
#endif
#else
#define RENDERER_API
#endif

using namespace std::literals;

/*
   this Rendering System is Left-Handed system where Z axis grows inside the screen
   Left-Handed Coordinate System (Z out of the screen)

           +Y (UP)         
              |   / +Z (in screen)  
              |  /
              | /
              o-------> +X (right)

   - X: left
   - Y: down
   - Z: into screen
*/
class RENDERER_API Renderer {
public:
    bool initializeWindow(bool fullscreen = false);
    bool setupWindow(const std::string& obj_file_path);
    void destroyWindow();
    void processInput();
    bool getWindowState();
    void update();
    void render(double timer_value);
    void drawText(std::string_view text, const vec2i_t& dims, const vec2i_t& pos);

private:
    void drawGrid();
    void drawPixel(int x, int y, uint32_t color);
    void drawRect(int x, int y, int width, int height, uint32_t color);
    void drawLine(int x0, int y0, int x1, int y1, uint32_t color);
    void drawTriangle(Triangle& tri, uint32_t color);
    void rasterizeTriangle(Triangle& tri, uint32_t color);
    void rasterizeFlatBottomTriangle(int x0, int y0, int x1, int y1, int x2, int y2,
                                     uint32_t color = 0xFFFF0000);
    void rasterizeFlatTopTriangle(int x0, int y0, int x1, int y1, int x2, int y2,
                                  uint32_t color = 0xFFFF0000);
    void renderColorBuffer();
    void clearColorBuffer(uint32_t color);
    void loadObjFileData(const std::string& obj_file_path);
    vec2f_t project(vec3f_t& point);

private:
    std::atomic_bool _isRunning = false;
    std::atomic_bool _pause{false};
    std::atomic_bool _enableFaceCulling{false};

    std::atomic_bool _wireframeModel{true};
    std::atomic_bool _VerticesModel{false};
    std::atomic_bool _raterizeModel{false};
    std::atomic_bool _processInput{false};

    bool _firstFrame{true};

    int _width{800};
    int _height{600};
    float _fovFactor = 640;  // for perspective projection
    //float _fovFactor = 128;  // for isometric projection

    uint32_t _previousFrameTime{0};
    const uint32_t _fps{30};
    const uint32_t _frameTargetTime{1000 / _fps};  //the time of one frame

    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> _windowPtr =
        std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>(nullptr, SDL_DestroyWindow);

    std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> _rendererPtr =
        std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)>(nullptr, SDL_DestroyRenderer);
    
    std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> _colorBufferTexturePtr =
        std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>(nullptr, SDL_DestroyTexture);
    
    TTF_Font* _ttfTextRenerer = nullptr;

    std::vector<uint32_t> _colorBuffer;
    std::vector<Triangle> _trianglesToRender;
    std::vector<Triangle> _lastTrianglesToRender;
    Mesh _mesh;

    vec3f_t _cameraPosition = {0, 0, -5};
    vec3f_t _rotation = {0.0, 0.0, 0.0};

    std::unique_ptr<std::thread> _processInputThread = nullptr;
    
    Timer _timer;
};