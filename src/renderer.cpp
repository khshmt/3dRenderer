//STL
#include <iostream>
//INTERNAL
#include <renderer.hpp>
#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif
bool Renderer::initializeWindow(bool fullscreen) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cerr << "Error initializing SDL\n";
        return false;
    }

    SDL_DisplayMode display_mode;
    auto success = SDL_GetCurrentDisplayMode(0, &display_mode);
    if (success == 0) {  //if zero returned that means success
        _windowWidth = display_mode.w;
        _windowHeight = display_mode.h;
        std::cout << "-Screen refersh_rate: " << display_mode.refresh_rate << " Hz\n";
        std::cout << "-Screen dims: " << _windowWidth << "x" << _windowHeight << '\n';
    }

    if (_windowPtr = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>(
            SDL_CreateWindow(nullptr, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _windowWidth,
                             _windowHeight, SDL_WINDOW_RESIZABLE),
            SDL_DestroyWindow);
        !_windowPtr) {
        std::cerr << "Error creating a window\n";
        return false;
    }

    if (_rendererPtr = std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)>(
            SDL_CreateRenderer(_windowPtr.get(), -1, 0), SDL_DestroyRenderer);
        !_rendererPtr) {
        std::cerr << "Error creating a renderer\n";
        return false;
    }

    if (fullscreen) {
        SDL_SetWindowFullscreen(_windowPtr.get(), SDL_WINDOW_MAXIMIZED);
    }

    if (TTF_Init() == 0) {
        if(std::filesystem::exists("Roboto-Regular.ttf") == false) {
            std::cerr << "Font file does not exist\n";
            return false;
        }
        _ttfTextRenerer = TTF_OpenFont("Roboto-Regular.ttf", 24);
    }

    return _isRunning = true;
}

bool Renderer::setupWindow(const std::string& obj_file_path) {
    _colorBuffer.reserve(_windowWidth * _windowHeight);

    if (_colorBufferTexturePtr = std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>(
            SDL_CreateTexture(_rendererPtr.get(), SDL_PIXELFORMAT_ABGR8888,
                              SDL_TextureAccess::SDL_TEXTUREACCESS_STREAMING, _windowWidth, _windowHeight),
            SDL_DestroyTexture);
        !_colorBufferTexturePtr) {
        std::cout << "falied to create Texture\n";
        return false;
    }

    constructProjectionMatrix(60.0, static_cast<float>(_windowHeight) / _windowWidth, 0.1, 100.0);

    auto objloaded = loadObjFileData(obj_file_path);
    auto png_file_path = std::filesystem::path(obj_file_path).replace_extension(".png");
    if (std::filesystem::exists(png_file_path)) {
        loadPNGTextureData(png_file_path.string());
    }
    _trianglesToRender.reserve(_mesh.faces.size());
    _lastTrianglesToRender.reserve(_mesh.faces.size());
    return objloaded;
}

void Renderer::drawPixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < _windowWidth && y >= 0 && y < _windowHeight) {
        _colorBuffer[(_windowWidth * y) + x] = color;
    }
}

void Renderer::drawTexel(const vec2i_t& point, const std::vector<uint32_t>& texture,
                         const Matrix<float, 4, 1>& a, const Matrix<float, 4, 1>& b,
                         const Matrix<float, 4, 1>& c, const vec2f_t& uv0, const vec2f_t& uv1,
                         const vec2f_t& uv2) {
    vec2f_t point_p{(float)point.x(), (float)point.y()};
    vec3f_t weights = barycentric_weights(vec2f_t{a.x(), a.y()}, vec2f_t{b.x(), b.y()},
                                          vec2f_t{c.x(), c.y()}, point_p);

    float alpha = weights.x();
    float beta = weights.y();
    float gamma = weights.z();

    // Variables to store the interpolated values of U, V, and also 1/w for the current pixel
    float interpolated_u;
    float interpolated_v;
    float interpolated_reciprocal_w;

    // Perform the interpolation of all U/w and V/w values using barycentric weights and a factor of 1/w
    // removed the distortion
    interpolated_u =
        (uv0.x() / a.w()) * alpha + (uv1.x() / b.w()) * beta + (uv2.x() / c.w()) * gamma;
    interpolated_v =
        (uv0.y() / a.w()) * alpha + (uv1.y() / b.w()) * beta + (uv2.y() / c.w()) * gamma;

    // Also interpolate the value of 1/w for the current pixel
    interpolated_reciprocal_w =
        (1 / a.w()) * alpha + (1 / b.w()) * beta + (1 / c.w()) * gamma;

    // Now we can divide back both interpolated values by 1/w
    interpolated_u /= interpolated_reciprocal_w;
    interpolated_v /= interpolated_reciprocal_w;

    // Map the UV coordinate to the full texture width and height
    int tex_x = abs((int)(interpolated_u * _textureWidth)) % _textureWidth;
    int tex_y = abs((int)(interpolated_v * _textureHeight)) % _textureHeight;

    drawPixel(point.x(), point.y(), texture[(_textureWidth * tex_y) + tex_x]);
}

vec3f_t Renderer::barycentric_weights(const vec2f_t& a, const vec2f_t& b, const vec2f_t& c,
                                      const vec2f_t& p) {
    // Find the vectors between the vertices ABC and point p
    auto ac = c - a;
    auto ab = b - a;
    auto ap = p - a;
    auto pc = c - p;
    auto pb = b - p;

    // Compute the area of the full parallegram/triangle ABC using 2D cross product
    float area_parallelogram_abc = (ac.x() * ab.y() - ac.y() * ab.x());  // || AC x AB ||

    // Alpha is the area of the small parallelogram/triangle PBC divided by the area of the full parallelogram/triangle ABC
    float alpha = (pc.x() * pb.y() - pc.y() * pb.x()) / area_parallelogram_abc;

    // Beta is the area of the small parallelogram/triangle APC divided by the area of the full parallelogram/triangle ABC
    float beta = (ac.x() * ap.y() - ac.y() * ap.x()) / area_parallelogram_abc;

    // Weight gamma is easily found since barycentric coordinates always add up to 1.0
    float gamma = 1 - alpha - beta;

    vec3f_t weights = {alpha, beta, gamma};
    return weights;
}

void Renderer::drawGrid() {
    for (uint32_t y{0}; y < _windowHeight; y += 20) {
        for (uint32_t x{0}; x < _windowWidth; x += 20) {
            _colorBuffer[(_windowWidth * y) + x] = 0xFFFFFFFF;
        }
    }
}

void Renderer::drawRect(int x, int y, int width, int height, uint32_t color) {
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            int current_x = x + i;
            int current_y = y + j;
            drawPixel(current_x, current_y, color);
        }
    }
}

// DDA(digital differential analyzer) line drawing algorithm
// there is also another faster algorithm called Bresenham's line algorithm
void Renderer::drawLine(int x0, int y0, int x1, int y1, uint32_t color) {
    int delta_x = (x1 - x0);
    int delta_y = (y1 - y0);

    float longest_side_length = (abs(delta_x) >= abs(delta_y)) ? static_cast<float>(abs(delta_x)) : static_cast<float>(abs(delta_y));

    float x_inc = delta_x / longest_side_length;
    float y_inc = delta_y / longest_side_length;

    float current_x = x0;
    float current_y = y0;

    for (int i = 0; i <= longest_side_length; i++) {
        drawPixel(static_cast<int>(current_x), static_cast<int>(current_y), color);
        current_x += x_inc;
        current_y += y_inc;
    }
}

void Renderer::drawTriangle(const Triangle& tri, uint32_t color) {
    drawLine(tri.points[0].x(), tri.points[0].y(), tri.points[1].x(), tri.points[1].y(), color);
    drawLine(tri.points[1].x(), tri.points[1].y(), tri.points[2].x(), tri.points[2].y(), color);
    drawLine(tri.points[2].x(), tri.points[2].y(), tri.points[0].x(), tri.points[0].y(), color);
}

//   flat bottom triangle
//  --------------> +x
//  |       (x0,y0)
//  |          / \
//  |         /   \
//  |        /     \
//  |       /       \
//  |      /         \
//  v +y (x1,y1)------(x2,y2)
void Renderer::rasterizeFlatBottomTriangle(const vec2i_t& p0, const vec2i_t& p1, const vec2i_t& p2,
                                           uint32_t color) {
    // Find the two inverse slopes (two triangle legs)
    // inverse slope = run / rise, which tells us how much x changes for each unit change in y
    // since we are looping over y (scanline by scanline), while slope = rise / run
    float inv_slope_1{};
    float inv_slope_2{};
    if (p1.y() - p0.y() != 0 && p2.y() - p0.y() != 0) {
        inv_slope_1 = static_cast<float>(p1.x() - p0.x()) / (p1.y() - p0.y());
        inv_slope_2 = static_cast<float>(p2.x() - p0.x()) / (p2.y() - p0.y());
    }

    // Start x_start and x_end from the top vertex (x0,y0)
    float x_start = p0.x();
    float x_end = p0.x();

    // Loop all the scanlines from top to bottom
    for (int y = p0.y(); y <= p2.y(); y++) {
        drawLine(x_start, y, x_end, y, color);
        x_start += inv_slope_1;
        x_end += inv_slope_2;
    }
}

//   flat top triangle
// -------------------> +x
// | (x0,y0)------(x1,y1)
// |     \         /
// |      \       /
// |       \     /
// |        \   /
// |         \ /
// |       (x2,y2)
// v +y
void Renderer::rasterizeFlatTopTriangle(const vec2i_t& p0, const vec2i_t& p1, const vec2i_t& p2,
                                        uint32_t color) {
    // Find the two slopes (two triangle legs)
    float inv_slope_1{};
    float inv_slope_2{};
    if (p2.y() - p0.y() != 0 && p2.y() - p1.y() != 0) {
        inv_slope_1 = static_cast<float>(p2.x() - p0.x()) / (p2.y() - p0.y());
        inv_slope_2 = static_cast<float>(p2.x() - p1.x()) / (p2.y() - p1.y());
    }

    // Start x_start and x_end from the bottom vertex (x2,y2)
    float x_start = p2.x();
    float x_end = p2.x();

    // Loop all the scanlines from bottom to top
    for (int y = p2.y(); y >= p0.y(); y--) {
        drawLine(x_start, y, x_end, y, color);
        x_start -= inv_slope_1;
        x_end -= inv_slope_2;
    }
}

//
//          (x0,y0)
//            / \
//           /   \
//          /     \
//         /       \
//        /         \
//   (x1,y1)------(Mx,My)
//       \_           \
//          \_         \
//             \_       \
//                \_     \
//                   \    \
//                     \_  \
//                        \_\
//                           \
//                         (x2,y2)
//
void Renderer::rasterizeTriangle(const Triangle& tri, uint32_t color) {
    std::array<vec2i_t, 3> verts = {
        vec2i_t{(int)tri.points[0].x(), (int)tri.points[0].y()},
        vec2i_t{(int)tri.points[1].x(), (int)tri.points[1].y()},
        vec2i_t{(int)tri.points[2].x(), (int)tri.points[2].y()},
    };

    // Sort by y, where y0 < y1 < y2
    std::sort(std::begin(verts), std::end(verts), [](auto& a, auto& b) { return a.y() < b.y(); });

    auto x0 = verts[0].x();
    auto y0 = verts[0].y();

    auto x1 = verts[1].x();
    auto y1 = verts[1].y();

    auto x2 = verts[2].x();
    auto y2 = verts[2].y();
    
    if (y1 == y2) {
        // Draw flat-bottom triangle
        rasterizeFlatBottomTriangle({x0, y0}, {x1, y1}, {x2, y2}, color);
    } else if (y0 == y1) {
        // Draw flat-top triangle
        rasterizeFlatTopTriangle({x0, y0}, {x1, y1}, {x2, y2}, color);
    } else {
        // Calculate the new vertex (Mx,My) using triangle similarity
        // Mx - x0      y1 - y0
        // --------- =  ---------  We need Mx
        //  x2 - x0     y2 - y0
        float factor = float(y1 - y0) / float(y2 - y0);

        float Mx = x0 + factor * (x2 - x0);
        float My = y1;

        // Draw flat-bottom triangle
        rasterizeFlatBottomTriangle({x0, y0}, {x1, y1}, {(int)Mx, (int)My}, color);
        // Draw flat-top triangle
        rasterizeFlatTopTriangle({x1, y1}, {(int)Mx, (int)My}, {x2, y2}, color);
    }
}

void Renderer::rasterizeTexturedTriangle(const Triangle& tri, const std::vector<uint32_t>& textureBuffer) {
    std::array<std::pair<Matrix<int, 4, 1>, vec2f_t>, 3> verts = {
        {{tri.points[0].toInt(), tri.text_coords[0]},
         {tri.points[1].toInt(), tri.text_coords[1]},
         {tri.points[2].toInt(), tri.text_coords[2]}}};

    // Sort by y, where y0 < y1 < y2
    std::sort(verts.begin(), verts.end(),
              [](auto& a, auto& b) { return a.first.y() < b.first.y(); });

    // p = point, t = texture coordinate
    auto& [p0, t0] = verts[0];
    auto& [p1, t1] = verts[1];
    auto& [p2, t2] = verts[2];

    auto [u0, v0] = t0.get2();
    auto [u1, v1] = t1.get2();
    auto [u2, v2] = t2.get2();

    auto [x0, y0, z0, w0] = p0.get4();
    auto [x1, y1, z1, w1] = p1.get4();
    auto [x2, y2, z2, w2] = p2.get4();

    // flip the V Component to account for inverted UV coordinates
    v0 = 1.0 - v0;
    v1 = 1.0 - v1;
    v2 = 1.0 - v2;

    Matrix<float, 4, 1> point_a = {(float)x0, (float)y0, (float)z0, (float)w0};
    Matrix<float, 4, 1> point_b = {(float)x1, (float)y1, (float)z1, (float)w1};
    Matrix<float, 4, 1> point_c = {(float)x2, (float)y2, (float)z2, (float)w2};
    vec2f_t a_uv = {u0, v0};
    vec2f_t b_uv = {u1, v1};
    vec2f_t c_uv = {u2, v2};

    ///////////////////////////////////////////////////////
    // Render the upper part of the triangle (flat-bottom)
    ///////////////////////////////////////////////////////
    float inv_slope_1 = 0;
    float inv_slope_2 = 0;

    if (y1 - y0 != 0)
        inv_slope_1 = (float)(x1 - x0) / abs(y1 - y0);
    if (y2 - y0 != 0)
        inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);

    if (y1 - y0 != 0) {
        for (int y = y0; y <= y1; y++) {
            int x_start = x1 + (y - y1) * inv_slope_1;
            int x_end = x0 + (y - y0) * inv_slope_2;

            if (x_end < x_start) {
                std::swap(x_start, x_end);  // swap if x_start is to the right of x_end
            }

            for (int x = x_start; x < x_end; x++) {
                // Draw our pixel with the color that comes from the texture
                drawTexel({x, y}, textureBuffer, point_a, point_b, point_c, a_uv, b_uv, c_uv);
            }
        }
    }
    
    ///////////////////////////////////////////////////////
    // Render the bottom part of the triangle (flat-top)
    ///////////////////////////////////////////////////////
    inv_slope_1 = 0;
    inv_slope_2 = 0;

    if (y2 - y1 != 0)
        inv_slope_1 = (float)(x2 - x1) / abs(y2 - y1);
    if (y2 - y0 != 0)
        inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);

    if (y2 - y1 != 0) {
        for (int y = y1; y <= y2; y++) {
            int x_start = x1 + (y - y1) * inv_slope_1;
            int x_end = x0 + (y - y0) * inv_slope_2;

            if (x_end < x_start) {
                std::swap(x_start, x_end);  // swap if x_start is to the right of x_end
            }

            for (int x = x_start; x < x_end; x++) {
                // Draw our pixel with the color that comes from the texture
                drawTexel({x, y}, textureBuffer, point_a, point_b, point_c, a_uv, b_uv, c_uv);
            }
        }
    }

}

void Renderer::renderColorBuffer() {
    SDL_UpdateTexture(_colorBufferTexturePtr.get(), nullptr, _colorBuffer.data(),
                      (int)(sizeof(uint32_t) * _windowWidth)  //Pitch ==> size of one row in bytes
    );

    SDL_RenderCopy(_rendererPtr.get(), _colorBufferTexturePtr.get(), nullptr, nullptr);
}

void Renderer::clearColorBuffer(uint32_t color) {
    for (uint32_t y{0}; y < _windowHeight; ++y) {
        for (uint32_t x{0}; x < _windowWidth; ++x) {
            _colorBuffer[(_windowWidth * y) + x] = color;
        }
    }
}

void Renderer::processInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {  // keep processing until queue is empty
        switch (event.type) {
            case SDL_QUIT:
                _isRunning = false;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        _isRunning = false;
                        break;
                    case SDLK_SPACE:
                        _pause = !_pause;
                        break;
                    case SDLK_c:
                        _enableFaceCulling = true;
                        break;
                    case SDLK_d:
                        _enableFaceCulling = false;
                        break;
                    case SDLK_1:
                        _currentRenderMode = RenderMode::WIREFRAME;
                        break;
                    case SDLK_2:
                        _currentRenderMode = RenderMode::WIREFRAME_VERTICES;
                        break;
                    case SDLK_3:
                        _currentRenderMode = RenderMode::RASTERIZE;
                        break;
                    case SDLK_4:
                        _currentRenderMode = RenderMode::RASTERIZE_WIREFRAME;
                        break;
                    case SDLK_5:
                        _currentRenderMode = RenderMode::TEXTURE;
                        break;
                    case SDLK_6:
                        _currentRenderMode = RenderMode::TEXTURE_WIREFRAME;
                        break;
                    default:
                        break;
                }
                break;

            case SDL_MOUSEWHEEL:
                if (event.wheel.y > 0) {
                    // Scroll up
                    _cameraPosition.z()++;  // increase your int member variable
                } else if (event.wheel.y < 0) {
                    // Scroll down
                    _cameraPosition.z()--;  // decrease your int member variable
                }
                break;

            default:
                break;
        }
    }
}

bool Renderer::getWindowState() {
    return _isRunning;
}

void Renderer::constructProjectionMatrix(float fov, float aspectRatio, float znear, float zfar) {
    // | aspectRatio*fovRad    0               0                                           0 |
    // | 0                    fovRad           0                                           0 |
    // | 0                    0               zfar/(zfar-znear)   (-zfar*znear)/(zfar-znear) |
    // | 0                    0               1                                            0 |
    float fovRad = 1.0f / tanf(fov * 0.5f / 180.0f * 3.14159f);
    _persProjMatrix(0, 0) = aspectRatio * fovRad;
    _persProjMatrix(1, 1) = fovRad;
    _persProjMatrix(2, 2) = zfar / (zfar - znear);
    _persProjMatrix(2, 3) = (-zfar * znear) / (zfar - znear);
    _persProjMatrix(3, 2) = 1.0f;
    _persProjMatrix(3, 3) = 0.0f;
}

Matrix<float, 4, 1> Renderer::project(vec3f_t& point) { 
    Matrix<float, 4, 1> vec =
        _persProjMatrix * Matrix<float, 4, 1>{point.x(), point.y(), point.z(), 1.0f};
    
    // TODO: Clipping against near plane should be done here
    //TODO
    
    // perform perspective divide
    // w_component = vec(3, 0) is the original Z value of the 3d point before projection
    if (vec(3, 0) != 0.0f) {
        vec(0, 0) /= vec(3, 0);
        vec(1, 0) /= vec(3, 0);
        vec(2, 0) /= vec(3, 0);
    }
    return vec;
}

uint32_t Renderer::calculateLightIntensityColor(uint32_t original_color, float percentage_factor) {
    if (percentage_factor < 0.0f)
        percentage_factor = 0.0f;
    if (percentage_factor > 1.0f)
        percentage_factor = 1.0f;
    uint32_t a = original_color & 0xFF000000;
    uint32_t r = (original_color & 0x00FF0000) * percentage_factor;
    uint32_t g = (original_color & 0x0000FF00) * percentage_factor;
    uint32_t b = (original_color & 0x000000FF) * percentage_factor;

    return (a | (r & 0x00FF0000) | (g & 0x0000FF00) | (b & 0x000000FF)); // new color
}

void Renderer::loadPNGTextureData(const std::string& fileName) {
    // Initialize SDL_image with PNG support if not already done
    static bool sdl_image_initialized = false;
    if (!sdl_image_initialized) {
        int flags = IMG_INIT_PNG;
        if ((IMG_Init(flags) & flags) != flags) {
            std::cerr << "Failed to initialize SDL_image: " << IMG_GetError() << std::endl;
            return;
        }
        sdl_image_initialized = true;
    }

    // Load the PNG as an SDL surface
    SDL_Surface* surface = IMG_Load(fileName.c_str());
    if (!surface) {
        std::cerr << "Failed to load PNG file: " << IMG_GetError() << std::endl;
        return;
    }

    // Ensure we have a consistent ABGR8888 pixel format
    SDL_Surface* converted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ABGR8888, 0);
    SDL_FreeSurface(surface);
    if (!converted) {
        std::cerr << "Failed to convert surface to RGBA32: " << SDL_GetError() << std::endl;
        return;
    }

    // Extract pixel data
    _textureWidth = converted->w;
    _textureHeight = converted->h;
    _meshTextureBuffer.resize(_textureWidth * _textureHeight);

    std::memcpy(_meshTextureBuffer.data(), converted->pixels,
                _textureWidth * _textureHeight * sizeof(uint32_t));

    SDL_FreeSurface(converted);
}

void Renderer::update() {
    _timer.startWatch(__func__);

    //this technique is prefered because SDL_TICKS_PASSED could lead to high CPU usage because of the while loop
    //auto delayTime = _frameTargetTime - (SDL_GetTicks() - _previousFrameTime);
    //if (delayTime > 0 && delayTime <= _frameTargetTime) {
    //    SDL_Delay(delayTime);
    //}

    // this technique is better for consistent frame rate, and no high CPU usage observed on windows
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), _previousFrameTime + _frameTargetTime)) {}
    _previousFrameTime = SDL_GetTicks();

    if (!_pause) {
        // Scale
        //_mesh.scale.x() += 0.02;
        //_mesh.scale.y() += 0.02;
        // Translation
        //_mesh.translation.x() += 0.04;
        _mesh.translation.z() = -_cameraPosition.z();
        // Roation
        _mesh.rotation.x() += 0.01;
        _mesh.rotation.y() += 0.01;
        _mesh.rotation.z() += 0.01;

        _worldMatrix.setEye();
        _worldMatrix.setScale(_mesh.scale.x(), _mesh.scale.y(), _mesh.scale.z());
        _worldMatrix.setTranslation(_mesh.translation.x(), _mesh.translation.y(),
                                             _mesh.translation.z());
        _worldMatrix.setRotation(_mesh.rotation.x(), _mesh.rotation.y(),
                                          _mesh.rotation.z());
        for (auto& face : _mesh.faces) {
            int i{0};
            std::array<vec3f_t, 3> face_vertices;
            face_vertices[0] = _mesh.vertices[face.a];
            face_vertices[1] = _mesh.vertices[face.b];
            face_vertices[2] = _mesh.vertices[face.c];

            for (auto& vertex : face_vertices) {
                Matrix<float, 4, 1> vec =
                    _worldMatrix * Matrix<float, 4, 1>{vertex.x(), vertex.y(), vertex.z(), 1};
                vertex.x() = vec(0, 0);
                vertex.y() = vec(1, 0);
                vertex.z() = vec(2, 0);
            }

            // Face CUlling Check
            auto [back_face, face_normal] = CullingCheck(face_vertices);
            face.normal = face_normal;
            if (_enableFaceCulling && back_face)
                    continue;
            // loop over face vertecies to perform projection
            Triangle projected_triangle;
            for (auto& vertex : face_vertices) {
                auto projected_point = project(vertex);
                // scale into view
                projected_point.x() *= _windowWidth / 2.0;   
                projected_point.y() *= _windowHeight / 2.0; 

                // invert y axis to account for flipped screen y coordinates
                projected_point.y() *= -1; 

                // translate to the center of the screen
                projected_point.x() += _windowWidth / 2.0;  
                projected_point.y() += _windowHeight / 2.0;
                projected_triangle.points[i++] = projected_point;

                projected_triangle.text_coords[0] = face.a_uv;
                projected_triangle.text_coords[1] = face.b_uv;
                projected_triangle.text_coords[2] = face.c_uv;
                
                projected_triangle.avg_depth = (face_vertices[0].z() + face_vertices[1].z() + face_vertices[2].z()) / 3.0f;
                projected_triangle.normal = face.normal;
                projected_triangle.color = face.color;
            }
            _trianglesToRender.emplace_back(projected_triangle);
        }
        // sort triangles from back to front based on average depth
        // C++ std::sort is an introspective sort algorithm, complexity O(n log(n))
        std::sort(std::begin(_trianglesToRender), std::end(_trianglesToRender),
                  [](const Triangle& t1, const Triangle& t2) { return t1.avg_depth > t2.avg_depth; });
        // store the last frame triangles, incase of pause is hit we can still render the last frame
        _lastTrianglesToRender = std::move(_trianglesToRender);
    }
    _timer.endWatch();
}

void Renderer::render(double timer_value) {
    _timer.startWatch(__func__);

    clearColorBuffer(0xFF000000);
    drawGrid();

    bool wireframe = _currentRenderMode == RenderMode::WIREFRAME ||
                     _currentRenderMode == RenderMode::RASTERIZE_WIREFRAME ||
                     _currentRenderMode == RenderMode::TEXTURE_WIREFRAME ||
                     _currentRenderMode == RenderMode::WIREFRAME_VERTICES; 

    bool raster = _currentRenderMode == RenderMode::RASTERIZE ||
                  _currentRenderMode == RenderMode::RASTERIZE_WIREFRAME;

    bool textured = _currentRenderMode == RenderMode::TEXTURE ||
                    _currentRenderMode == RenderMode::TEXTURE_WIREFRAME;

    bool showVertices = _currentRenderMode == RenderMode::WIREFRAME_VERTICES;

    for (auto& triangle : _lastTrianglesToRender) {
        uint32_t wireframe_color{0xFF00FF00};  // default wirferame color is green
        if (raster) {
            auto light_intensity_factor = -(triangle.normal * _lightDirection);
            auto color = calculateLightIntensityColor(triangle.color, light_intensity_factor);
            rasterizeTriangle(triangle, color);
            wireframe_color = 0xFF000000;  // black
        }
        if (textured && !_meshTextureBuffer.empty()) {
            rasterizeTexturedTriangle(triangle, _meshTextureBuffer);
            wireframe_color = 0xFF000000;  // black
        } 
        if (showVertices) {
            // draw red vertices
            drawRect(triangle.points[0].x(), triangle.points[0].y(), 3, 3, 0xFF0000FF);
            drawRect(triangle.points[1].x(), triangle.points[1].y(), 3, 3, 0xFF0000FF);
            drawRect(triangle.points[2].x(), triangle.points[2].y(), 3, 3, 0xFF0000FF);
        }
        if (wireframe) {
            drawTriangle(triangle, wireframe_color);
        }
    }
    renderColorBuffer();

    static std::string timer_value_ = std::move(std::to_string(timer_value));
    static auto t1 = std::chrono::steady_clock::now().time_since_epoch();
    auto t2 = std::chrono::steady_clock::now().time_since_epoch();
    if (t2 - t1 > 0.5s) {
        timer_value_ = std::move(std::to_string(timer_value));
        t1 = t2;
    }
    const vec2i_t dims1{100, 30};
    drawText(std::string("fps: "s + timer_value_), dims1, {(_windowWidth - dims1.x()) / 2, 40},
             stoi(timer_value_) < _fps ? false : true);

    const vec2i_t dims2{40, 30};
    drawText("Profiles: ", {100, 30}, {40, 40}, false);
    drawText(" #1 ", dims2, {40, 70}, _currentRenderMode == RenderMode::WIREFRAME);
    drawText(" #2 ", dims2, {40, 100}, _currentRenderMode == RenderMode::WIREFRAME_VERTICES);
    drawText(" #3 ", dims2, {40, 130}, _currentRenderMode == RenderMode::RASTERIZE);
    drawText(" #4 ", dims2, {40, 160}, _currentRenderMode == RenderMode::RASTERIZE_WIREFRAME);
    drawText(" #5 ", dims2, {40, 190}, _currentRenderMode == RenderMode::TEXTURE);
    drawText(" #6 ", dims2, {40, 220}, _currentRenderMode == RenderMode::TEXTURE_WIREFRAME);
    drawText("C_Key: Culling.", {150, 30}, {40, 260},  _enableFaceCulling);
    drawText("D_Key: Disable Culling.", {200, 30}, {40, 290}, !_enableFaceCulling);
    drawText("Space_Key: Pause.", {200, 30}, {40, 320}, _pause);
    drawText("Mouse Wheel Zoom in/out.", {200, 30}, {40, 350}, false);

    SDL_RenderPresent(_rendererPtr.get());
    _trianglesToRender.clear();
    _timer.endWatch();
}

bool Renderer::loadObjFileData(const std::string& obj_file_path) {
    FILE* file = fopen(obj_file_path.c_str(), "r");
    if (!file) {
        std::cerr << "Error opening file: " << obj_file_path << '\n';
        return false;
    }
    char line[1024];

    std::vector<vec2f_t> textureCoords;

    while (fgets(line, 1024, file)) {
        vec3f_t vertex;
        // Vertex information
        if (strncmp(line, "v ", 2) == 0) {
            if (sscanf(line, "v %f %f %f", &vertex.x(), &vertex.y(), &vertex.z()) != 3) {
                std::cerr << "Error parsing vertex line: " << line << '\n';
                continue;
            }
            _mesh.vertices.push_back(vertex);
        }

        vec2f_t textureCoord;
        //texture coordinates information
        if (strncmp(line, "vt ", 3) == 0) {
            if (sscanf(line, "vt %f %f", &textureCoord.x(), &textureCoord.y()) != 2) {
                std::cerr << "Error parsing texture coordinates line: " << line << '\n';
                continue;
            }
            textureCoords.push_back(textureCoord);
        }

        //int indices[9];  // 0-vertex_index, 1-texture_index, 2-normal_index,........,6-vertex_index, 7-texture_index, 8-normal_index
        int vertex_indicies[3];
        int texture_indicies[3];
        int normal_indicies[3];
        // Face information
        if (strncmp(line, "f ", 2) == 0) {
            if (sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d", &vertex_indicies[0],
                       &texture_indicies[0], &normal_indicies[0], &vertex_indicies[1],
                       &texture_indicies[1], &normal_indicies[1], &vertex_indicies[2],
                       &texture_indicies[2], &normal_indicies[2]) != 9) {
                std::cerr << "Error parsing face line: " << line << '\n';
                continue;
            }
            Face face = {.a_uv = textureCoords[texture_indicies[0] - 1],
                         .b_uv = textureCoords[texture_indicies[1] - 1],
                         .c_uv = textureCoords[texture_indicies[2] - 1],
                         .a = vertex_indicies[0] - 1,
                         .b = vertex_indicies[1] - 1,
                         .c = vertex_indicies[2] - 1,
                         .color = 0xFFFFFFFF};
            _mesh.faces.push_back(face);
        }
    }
    normalizeModel(_mesh.vertices);
    return 0 == fclose(file);
}

std::pair<bool, vec3f_t> Renderer::CullingCheck(const std::array<vec3f_t, 3>& face_vertices) {
    auto vec_a = face_vertices[0];
    auto vec_b = face_vertices[1];
    auto vec_c = face_vertices[2];

    // step1 calculate ab vector and ac vector
    auto vec_ab = vec_b - vec_a;
    auto vec_ac = vec_c - vec_a;

    // step2 calculate the normal
    auto vec_face_normal = vec_ab ^ vec_ac;
    // this line is important for lighting calculation, otherwise the light intensity will be wrong
    vec_face_normal.normalize();  

    //step3 find the camera ray (vector between camera origin and a point in the triangle)
    auto vec_camera_ray = _cameraPosition - vec_a;

    //step4 check how aligned the camera ray with face normal, if angle is less than 90 degree then we are looking at the back face
    if (vec_face_normal * vec_camera_ray < 0)  // zero means cos(90), less than zero means more than cos(90) degree angle
        return {true, vec_face_normal};  // back face

    return {false, vec_face_normal};  // front face
}

void Renderer::normalizeModel(std::vector<vec3f_t>& vertices) {
    if (vertices.empty())
        return;

    // Step 1: compute bounding box
    vec3f_t min = {FLT_MAX, FLT_MAX, FLT_MAX};
    vec3f_t max = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

    for (const auto& v : vertices) {
        min.x() = std::min(min.x(), v.x());
        min.y() = std::min(min.y(), v.y());
        min.z() = std::min(min.z(), v.z());

        max.x() = std::max(max.x(), v.x());
        max.y() = std::max(max.y(), v.y());
        max.z() = std::max(max.z(), v.z());
    }

    // Step 2: compute center and extent
    vec3f_t center = {(min.x() + max.x()) / 2.0f, (min.y() + max.y()) / 2.0f, (min.z() + max.z()) / 2.0f};

    vec3f_t size = {max.x() - min.x(), max.y() - min.y(), max.z() - min.z()};
    float maxExtent = std::max({size.x(), size.y(), size.z()});

    // Step 3: normalize vertices
    for (auto& v : vertices) {
        // translate to origin
        v.x() -= center.x();
        v.y() -= center.y();
        v.z() -= center.z();

        // scale to fit into unit cube [-0.5, 0.5]
        v.x() /= maxExtent;
        v.y() /= maxExtent;
        v.z() /= maxExtent;
    }
}

void Renderer::destroyWindow() {
    // SDL_Quit();
}

void Renderer::drawText(std::string_view text, const vec2i_t& dims, const vec2i_t& pos,
                        bool enabledMode) {
    SDL_Color color{};
    if (enabledMode)
        color = {0, 255, 0};
    else 
        color = {255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Blended(_ttfTextRenerer, text.data(), color);
    if (surface == nullptr) {
        std::cout << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << '\n';
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(_rendererPtr.get(), surface);
    if (texture == nullptr) {
        SDL_FreeSurface(surface);
        std::cout << "Unable to create texture from rendered text! SDL Error: " << SDL_GetError()
                  << '\n';
        return;
    }

    SDL_Rect renderQuad = {pos.x(), pos.y(), dims.x(), dims.y()};
    SDL_RenderCopy(_rendererPtr.get(), texture, nullptr, &renderQuad);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}