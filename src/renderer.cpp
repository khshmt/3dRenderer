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
        _width = display_mode.w;
        _height = display_mode.h;
        std::cout << "-Screen refersh_rate: " << display_mode.refresh_rate << " Hz\n";
        std::cout << "-Screen dims: " << _width << "x" << _height << '\n';
    }

    if (_windowPtr = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>(
            SDL_CreateWindow(nullptr, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _width,
                             _height, SDL_WINDOW_RESIZABLE),
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

bool Renderer:: setupWindow(const std::string& obj_file_path) {
    _colorBuffer.resize(_width * _height);

    if(_colorBufferTexturePtr = std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>(
        SDL_CreateTexture(_rendererPtr.get(), SDL_PIXELFORMAT_ARGB8888, SDL_TextureAccess::SDL_TEXTUREACCESS_STREAMING,
                          _width, _height),
        SDL_DestroyTexture); !_colorBufferTexturePtr) {
        std::cout << "falied to create Texture\n";
        return false;
    }

    constructProjectionMatrix(60, static_cast<float>(_height)/_width, 0.01, 100.0);

    loadObjFileData(obj_file_path);
    return true;
}

void Renderer::drawPixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < _width && y >= 0 && y < _height) {
        _colorBuffer[(_width * y) + x] = color;
    }
}

void Renderer::drawGrid() {
    for (uint32_t y{0}; y < _height; y += 20) {
        for (uint32_t x{0}; x < _width; x += 20) {
            _colorBuffer[(_width * y) + x] = 0xFFFFFFFF;
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

void Renderer::drawTriangle(Triangle& tri, uint32_t color) {
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
void Renderer::rasterizeFlatBottomTriangle(int x0, int y0, int x1, int y1, int x2,
                                           int y2, uint32_t color) {
    // Find the two inverse slopes (two triangle legs)
    // inverse slope = run / rise, which tells us how much x changes for each unit change in y
    // since we are looping over y (scanline by scanline), while slope = rise / run
    float inv_slope_1 = static_cast<float>(x1 - x0) / (y1 - y0);
    float inv_slope_2 = static_cast<float>(x2 - x0) / (y2 - y0);
                     
    // Start x_start and x_end from the top vertex (x0,y0)
    float x_start = x0;
    float x_end = x0;

    // Loop all the scanlines from top to bottom
    for (int y = y0; y <= y2; y++) {
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
void Renderer::rasterizeFlatTopTriangle(int x0, int y0, int x1, int y1, int x2, int y2,
                                        uint32_t color) {
    // Find the two slopes (two triangle legs)
    float inv_slope_1 = static_cast<float>(x2 - x0) / (y2 - y0);
    float inv_slope_2 = static_cast<float>(x2 - x1) / (y2 - y1);

    // Start x_start and x_end from the bottom vertex (x2,y2)
    float x_start = x2;
    float x_end = x2;

    // Loop all the scanlines from bottom to top
    for (int y = y2; y >= y0; y--) {
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
void Renderer::rasterizeTriangle(Triangle& tri, uint32_t color) {
    std::array<std::pair<int, int>, 3> verts = {{{tri.points[0].x(), tri.points[0].y()},
                                                 {tri.points[1].x(), tri.points[1].y()},
                                                 {tri.points[2].x(), tri.points[2].y()}}};

    // Sort by y, where y0 < y1 < y2
    std::sort(verts.begin(), verts.end(), [](auto& a, auto& b) { return a.second < b.second; });

    auto [x0, y0] = verts[0];
    auto [x1, y1] = verts[1];
    auto [x2, y2] = verts[2];

    if (y1 == y2) {
        // Draw flat-bottom triangle
        rasterizeFlatBottomTriangle(x0, y0, x1, y1, x2, y2, color);
    } else if (y0 == y1) {
        // Draw flat-top triangle
        rasterizeFlatTopTriangle(x0, y0, x1, y1, x2, y2, color);
    } else {
        // Calculate the new vertex (Mx,My) using triangle similarity
        // Mx - x0      y1 - y0
        // --------- =  ---------  Wen need Mx
        //  x2 - x0     y2 - y0
        int My = y1;
        int Mx = (((x2 - x0) * (y1 - y0)) / (y2 - y0)) + x0; 
        // Draw flat-bottom triangle
        rasterizeFlatBottomTriangle(x0, y0, x1, y1, Mx, My, color);

        // Draw flat-top triangle
        rasterizeFlatTopTriangle(x1, y1, Mx, My, x2, y2, color);
    }
}

void Renderer::renderColorBuffer() {
    SDL_UpdateTexture(_colorBufferTexturePtr.get(), nullptr, _colorBuffer.data(),
                      (int)(sizeof(uint32_t) * _width)  //Pitch ==> size of one row in bytes
    );

    SDL_RenderCopy(_rendererPtr.get(), _colorBufferTexturePtr.get(), nullptr, nullptr);
}

void Renderer::clearColorBuffer(uint32_t color) {
    for (uint32_t y{0}; y < _height; ++y) {
        for (uint32_t x{0}; x < _width; ++x) {
            _colorBuffer[(_width * y) + x] = color;
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
                        _wireframeModel = true;
                        _VerticesModel = true;
                        _raterizeModel = false;
                        break;
                    case SDLK_2:
                        _wireframeModel = true;
                        _VerticesModel = false;
                        _raterizeModel = false;
                        break;
                    case SDLK_3:
                        _wireframeModel = false;
                        _VerticesModel = false;
                        _raterizeModel = true;
                        break;
                    case SDLK_4:
                        _wireframeModel = true;
                        _VerticesModel = false;
                        _raterizeModel = true;
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

vec2f_t Renderer::project(vec3f_t& point) { 
    Matrix<float, 4, 1> vec =
        _persProjMatrix * Matrix<float, 4, 1>{point.x(), point.y(), point.z(), 1.0f};
    
    // TODO: Clipping against near plane should be done here
    //TODO
    
    // perform perspective divide
    //w_component = vec(3, 0) is the original Z value of the 3d point before projection
    if (vec(3, 0) != 0.0f) {
        vec(0, 0) /= vec(3, 0);
        vec(1, 0) /= vec(3, 0);
        vec(2, 0) /= vec(3, 0);
    }
    return {vec(0, 0), vec(1, 0)};
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
        _mesh.scale.x() += 0.02;
        _mesh.scale.y() += 0.02;
        // Translation
        _mesh.translation.x() += 0.04;
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
        for (const auto& face : _mesh.faces) {
            int i{0};
            std::array<vec3f_t, 3> face_vertices;
            face_vertices[0] = _mesh.vertices[face.a - 1];
            face_vertices[1] = _mesh.vertices[face.b - 1];
            face_vertices[2] = _mesh.vertices[face.c - 1];

            for (auto& vertex : face_vertices) {
                Matrix<float, 4, 1> vec =
                    _worldMatrix * Matrix<float, 4, 1>{vertex.x(), vertex.y(), vertex.z(), 1};
                vertex.x() = vec(0, 0);
                vertex.y() = vec(1, 0);
                vertex.z() = vec(2, 0);
            }

            // Face CUlling Check
            if (_enableFaceCulling && CullingCheck(face_vertices))
                    continue;
            // loop over face vertecies to perform projection
            Triangle projected_triangle;
            for (auto& vertex : face_vertices) {
                auto projected_point = project(vertex);
                projected_point.x() *= _width / 2.0;   // sclae
                projected_point.y() *= _height / 2.0;  // scale

                projected_point.x() += _width / 2.0;   // translate
                projected_point.y() += _height / 2.0;  // translate
                projected_triangle.points[i++] = projected_point;

                projected_triangle.avg_depth = (face_vertices[0].z() + face_vertices[1].z() + face_vertices[2].z()) / 3.0f;
            }
            _trianglesToRender.push_back(projected_triangle);
        }
        // sort triangles from back to front based on average depth
        // C++ std::sort is an introspective sort algorithm, which is a hybrid sorting algorithm
        // complexity O(n log(n))
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

    for (auto& triangle : _lastTrianglesToRender) {
        if (_VerticesModel) {
            drawRect(triangle.points[0].x(), triangle.points[0].y(), 3, 3, 0xFFFF0000);
            drawRect(triangle.points[1].x(), triangle.points[1].y(), 3, 3, 0xFFFF0000);
            drawRect(triangle.points[2].x(), triangle.points[2].y(), 3, 3, 0xFFFF0000);
        }
        if (_raterizeModel) {
            rasterizeTriangle(triangle, 0xFFFFFFFF);
        }
        if (_wireframeModel) {
            uint32_t color;
            if (_raterizeModel)
                color = 0xFF000000;
            else
                color = 0xFF00FF00;
            drawTriangle(triangle, color);
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
    drawText(std::string("fps: "s + timer_value_), dims1,
             {(_width - dims1.x()) / 2, 40});

    const vec2i_t dims2{40, 30};
    drawText("Profiles: ", {100, 30}, { 40, 40 });
    drawText(" #1 ", dims2, {40, 70});
    drawText(" #2 ", dims2, {40, 100});
    drawText(" #3 ", dims2, {40, 130});
    drawText(" #4 ", dims2, {40, 160});
    drawText("C_Key: Culling.", {150, 30}, {40, 220});
    drawText("D_Key: Disable Culling.", {200, 30}, {40, 250});
    drawText("Space_Key: Pause.", {200, 30}, {40, 280});
    drawText("Mouse Wheel Zoom in/out.", {200, 30}, {40, 310});

    SDL_RenderPresent(_rendererPtr.get());
    _trianglesToRender.clear();
    _timer.endWatch();
}

void Renderer::loadObjFileData(const std::string& obj_file_path) {
    FILE* file = fopen(obj_file_path.c_str(), "r");
    if (!file) {
        std::cerr << "Error opening file: " << obj_file_path << '\n';
        return;
    }
    char line[1024];

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
            Face face = {.a = vertex_indicies[0], .b = vertex_indicies[1], .c = vertex_indicies[2]};
            _mesh.faces.push_back(face);
        }
    }
    fclose(file);
    normalizeModel(_mesh.vertices);
}

bool Renderer::CullingCheck(std::array<vec3f_t, 3>& face_vertices) {
    auto vec_a = face_vertices[0];
    auto vec_b = face_vertices[1];
    auto vec_c = face_vertices[2];

    // step1 calculate ab vector and ac vector
    auto vec_ab = vec_b - vec_a;
    auto vec_ac = vec_c - vec_a;

    // step2 calculate the normal
    auto vec_face_normal = vec_ab ^ vec_ac;
    vec_face_normal.normalize();  // we could get rid of that line in case of just culling

    //step3 find the camera ray (vector between camera origin and a point in the triangle)
    auto vec_camera_ray = _cameraPosition - vec_a;

    //step4 check how aligned the camera ray with face normal, if angle is less than 90 degree then we are looking at the back face
    if (vec_face_normal * vec_camera_ray < 0.0f)  // zero means cos(90), less than zero means more than cos(90) degree angle
        return true;  // back face

    return false;  // front face
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

void Renderer::drawText(std::string_view text, const vec2i_t& dims, const vec2i_t& pos) {
    SDL_Surface* surface = TTF_RenderText_Blended(_ttfTextRenerer, text.data(), {255,255,255});
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