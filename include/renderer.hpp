#pragma once
// stl
#include <atomic>
#include <memory>
#include <thread>
#include <vector>
#include <string>
#include <execution>
// internal
#include "Mesh.hpp"
#include "timer.hpp"
#include "helperFuncs.hpp"
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

using Eigen::Vector2f;
using Eigen::Vector3f;
using Eigen::Vector4f;

using Eigen::Vector2i;
using Eigen::Vector3i;
using Eigen::Vector4i;


enum class RenderMode {
    WIREFRAME,
    WIREFRAME_VERTICES,
    RASTERIZE,                
    RASTERIZE_WIREFRAME,
    TEXTURE,
    TEXTURE_WIREFRAME
};

enum FRUSTUMPLANES {
    LEFT_PLANE = 0,
    RIGHT_PLANE,
    TOP_PLANE,
    BOTTOM_PLANE,
    NEAR_PLANE,
    FAR_PLANE
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
    void drawText(std::string_view text, const Vector2i& dims, const Vector2i& pos,
                  bool enabledMode);
    void drawGrid();
    void drawPixel(int x, int y, uint32_t color);
    void drawTrianglePixel(int x, int y, uint32_t color, const Vector4f& point_a,
                           const Vector4f& point_b, const Vector4f& point_c);
    void drawTexel(const Vector2i& point, const std::vector<uint32_t>& texture, const Vector4f& a,
                   const Vector4f& b, const Vector4f& c, const Vector2f& uv0, const Vector2f& uv1,
                   const Vector2f& uv2);
    Vector3f barycentric_weights(const Vector2f& a, const Vector2f& b, const Vector2f& c,
                                 const Vector2f& p);
    void drawRect(int x, int y, int width, int height, uint32_t color);
    void drawLine(int x0, int y0, int x1, int y1, uint32_t color);
    void drawTriangle(const Triangle& tri, uint32_t color);
    void rasterizeTexturedTriangle(const Triangle& tri, const std::vector<uint32_t>& textureBuffer);
    void rasterizeTriangle1(const Triangle& tri, uint32_t color);
    void rasterizeTriangle2(const Triangle& tri, uint32_t color);
    void rasterizeFlatBottomTriangle(const Vector2i& p0, const Vector2i& p1, const Vector2i& p2,
                                     uint32_t color);
    void rasterizeFlatTopTriangle(const Vector2i& p0, const Vector2i& p1, const Vector2i& p2,
                                  uint32_t color);
    Eigen::Matrix4f lookAt(const Vector3f& eye, const Vector3f& target, const Vector3f& up);
    void renderColorBuffer();
    void clearColorBuffer(uint32_t color);
    void normalizeModel(std::vector<Vector3f>& vertices);
    std::pair<bool, Vector3f> CullingCheck(const std::array<Vector3f, 3>& face_vertices);
    void constructProjectionMatrix(float fov, float aspectRatio, float znear, float zfar);
    void initializeFrustumPlanes(float fovX, float fovY, float zNear, float zFar);
    Polygon createPolygon(const Vector3f& a, const Vector3f& b, const Vector3f& c,
                          const Vector2f& a_uv, const Vector2f& b_uv, const Vector2f& c_uv);
    std::vector<Triangle> trianglesFromPolygons(const Polygon& polygon);
    void clipPolygon(Polygon& polygon);
    void clipPolygonAgainstPlane(Polygon& polygon, FRUSTUMPLANES plane);
    Vector4f project(Vector4f& point);
    uint32_t calculateLightIntensityColor(uint32_t original_color, float percentage_factor);
    bool loadObjFileData(const std::string& obj_file_path);
    void loadPNGTextureData(const std::string& fileName);

private:
    Mesh _mesh;

    Eigen::Matrix<float, 4, 4> _worldMatrix = Eigen::Matrix4f::Identity();
    Eigen::Matrix<float, 4, 4> _persProjMatrix = Eigen::Matrix4f::Zero();
    Eigen::Matrix<float, 4, 4> _viewMatrix = Eigen::Matrix4f::Identity(); // view/camera matrix

    struct Camera {
        Vector3f _position = {0.0f, 0.0f, -2.0f};
        Vector3f _direction = {0.0f, 0.0f, 1.0f};
        Vector3f _forwardVelocity = {0.0f, 0.0f, 0.0f};
        float _yaw{0.0};  // roation around y axis ofthe camera
        float _pitch{0.0};  // rotation around x axis of the camera
    } _camera;
 
    struct FrustumPlane {
        Vector3f _point;
        Vector3f _normal;
    };

    std::vector<Triangle> _trianglesToRender;
    std::vector<Triangle> _lastTrianglesToRender;
    std::vector<uint32_t> _colorBuffer;
    std::vector<uint32_t> _meshTextureBuffer;
    std::vector<float> _zBuffer;
    std::vector<float> _zBufferAlternative;
    std::array<FrustumPlane, 6> frustumPlanes;

    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> _windowPtr =
        std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>(nullptr, SDL_DestroyWindow);

    std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> _rendererPtr =
        std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)>(nullptr, SDL_DestroyRenderer);
    
    std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> _colorBufferTexturePtr =
        std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>(nullptr, SDL_DestroyTexture);


    Vector3f _lightDirection = {0.0, 0.0, 1.0};

    TTF_Font* _ttfTextRenerer = nullptr;

    int _windowWidth{};
    int _windowHeight{};
    int _textureWidth{};
    int _textureHeight{};

    uint32_t _previousFrameTime{0};
    const uint32_t _fps{60};
    const uint32_t _frameTargetTime{1000 / _fps};  //the time of one frame
    float _deltaTime{};

    RenderMode _currentRenderMode = RenderMode::WIREFRAME;
    SDL_Color _renderModeTextColor = {255, 255, 255, 255};

    bool _isRunning = false;
    bool _pause{false};
    bool _enableFaceCulling{true};
    bool _rotateModel{false};
};