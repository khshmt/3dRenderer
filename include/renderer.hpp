#pragma once
// stl
#include <atomic>
#include <memory>
#include <thread>
#include <vector>
#include <string>
// internal
#include "Mesh.hpp"
#include "timer.hpp"
#include "vector.hpp"
#include "matrix.hpp"
// internal thirdparty
#include "upng.h"
// 3rd-Party_Libs
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

enum class RenderMode {
    WIREFRAME,
    WIREFRAME_VERTICES,
    RASTERIZE,                
    RASTERIZE_WIREFRAME,
    TEXTURE,
    TEXTURE_WIREFRAME
};

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
    void drawText(std::string_view text, const vec2i_t& dims, const vec2i_t& pos, bool enabledMode);

private:
    void drawGrid();
    void drawPixel(int x, int y, uint32_t color);
    void drawTexel(vec2i_t point, const std::vector<uint32_t> texture, Matrix<float, 4, 1> a,
                   Matrix<float, 4, 1> b, Matrix<float, 4, 1> c, vec2f_t uv0, vec2f_t uv1,
                   vec2f_t uv2);
    vec3f_t barycentric_weights(vec2f_t a, vec2f_t b, vec2f_t c, vec2f_t p);
    void drawRect(int x, int y, int width, int height, uint32_t color);
    void drawLine(int x0, int y0, int x1, int y1, uint32_t color);
    void drawTriangle(Triangle& tri, uint32_t color);
    void rasterizeTexturedTriangle(Triangle& tri, const std::vector<uint32_t>& textureBuffer);
    void rasterizeTriangle(Triangle& tri, uint32_t color);
    void rasterizeFlatBottomTriangle(vec2i_t p0, vec2i_t p1, vec2i_t p2, uint32_t color);
    void rasterizeFlatTopTriangle(vec2i_t p0, vec2i_t p1, vec2i_t p2, uint32_t color);
    void renderColorBuffer();
    void clearColorBuffer(uint32_t color);
    void normalizeModel(std::vector<vec3f_t>& vertices);
    std::pair<bool, vec3f_t> CullingCheck(std::array<vec3f_t, 3>& face_vertices);
    void constructProjectionMatrix(float fov, float aspectRatio, float znear, float zfar);
    Matrix<float, 4, 1> project(vec3f_t& point);
    uint32_t calculateLightIntensityColor(uint32_t original_color, float percentage_factor);
    bool loadObjFileData(const std::string& obj_file_path);
    void loadPNGTextureData(const std::string& fileName);


private:
    std::atomic_bool _isRunning = false;
    std::atomic_bool _pause{false};
    std::atomic_bool _enableFaceCulling{true};

    bool _firstFrame{true};

    int _width{800};
    int _height{600};

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

    Matrix<float, 4, 4> _worldMatrix;
    Matrix<float, 4, 4> _persProjMatrix;  // perspective projection matrix
    vec3f_t _cameraPosition = {0, 0, -5};
    vec3f_t _lightDirection = {0, 0, 1};
    RenderMode _currentRenderMode = RenderMode::WIREFRAME;
    SDL_Color _renderModeTextColor = {255, 255, 255, 255};
    int _textureWidth;
    int _textureHeight;
    std::vector<uint32_t> _meshTextureBuffer;
    std::unique_ptr<upng_t, decltype(&upng_free)> _pngTexture{nullptr, upng_free};
    Timer _timer;
};