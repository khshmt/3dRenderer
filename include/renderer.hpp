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
// 3rd-Party_Libs
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>


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
    bool getWindowState();
    void processInput();
    void update();
    void render(double timer_value);
    void destroyWindow();

private:
    void drawText(std::string_view text, const vec2i_t& dims, const vec2i_t& pos, bool enabledMode);
    void drawGrid();
    void drawPixel(int x, int y, uint32_t color);
    void drawTrianglePixel(int x, int y, uint32_t color, const Matrix<float, 4, 1>& point_a,
                           const Matrix<float, 4, 1>& point_b, const Matrix<float, 4, 1>& point_c);
    void drawTexel(const vec2i_t& point, const std::vector<uint32_t>& texture,
                   const Matrix<float, 4, 1>& a, const Matrix<float, 4, 1>& b,
                   const Matrix<float, 4, 1>& c, const vec2f_t& uv0, const vec2f_t& uv1,
                   const vec2f_t& uv2);
    vec3f_t barycentric_weights(const vec2f_t& a, const vec2f_t& b, const vec2f_t& c,
                                const vec2f_t& p);
    void drawRect(int x, int y, int width, int height, uint32_t color);
    void drawLine(int x0, int y0, int x1, int y1, uint32_t color);
    void drawTriangle(const Triangle& tri, uint32_t color);
    void rasterizeTexturedTriangle(const Triangle& tri, const std::vector<uint32_t>& textureBuffer);
    void rasterizeTriangle(const Triangle& tri, uint32_t color);
    void rasterizeFlatBottomTriangle(const vec2i_t& p0, const vec2i_t& p1, const vec2i_t& p2,
                                     uint32_t color);
    void rasterizeFlatTopTriangle(const vec2i_t& p0, const vec2i_t& p1, const vec2i_t& p2,
                                  uint32_t color);
    void renderColorBuffer();
    void clearColorBuffer(uint32_t color);
    void normalizeModel(std::vector<vec3f_t>& vertices);
    std::pair<bool, vec3f_t> CullingCheck(const std::array<vec3f_t, 3>& face_vertices);
    void constructProjectionMatrix(float fov, float aspectRatio, float znear, float zfar);
    Matrix<float, 4, 1> project(vec3f_t& point);
    uint32_t calculateLightIntensityColor(uint32_t original_color, float percentage_factor);
    bool loadObjFileData(const std::string& obj_file_path);
    void loadPNGTextureData(const std::string& fileName);


private:
    Timer _timer;
    Mesh _mesh;

    Matrix<float, 4, 4> _worldMatrix;
    Matrix<float, 4, 4> _persProjMatrix;  // perspective projection matrix

    std::vector<Triangle> _trianglesToRender;
    std::vector<Triangle> _lastTrianglesToRender;
    std::vector<uint32_t> _colorBuffer;
    std::vector<uint32_t> _meshTextureBuffer;
    std::vector<float> _zBuffer;

    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> _windowPtr =
        std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>(nullptr, SDL_DestroyWindow);

    std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> _rendererPtr =
        std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)>(nullptr, SDL_DestroyRenderer);
    
    std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> _colorBufferTexturePtr =
        std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>(nullptr, SDL_DestroyTexture);

    vec3f_t _cameraPosition = {0, 0, -5};
    vec3f_t _lightDirection = {0, 0, 1};

    TTF_Font* _ttfTextRenerer = nullptr;

    int _windowWidth{};
    int _windowHeight{};
    int _textureWidth{};
    int _textureHeight{};

    uint32_t _previousFrameTime{0};
    const uint32_t _fps{60};
    const uint32_t _frameTargetTime{1000 / _fps};  //the time of one frame

    RenderMode _currentRenderMode = RenderMode::WIREFRAME;
    SDL_Color _renderModeTextColor = {255, 255, 255, 255};

    bool _isRunning = false;
    bool _pause{false};
    bool _enableFaceCulling{true};
};