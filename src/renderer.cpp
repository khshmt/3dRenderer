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

void Renderer::drawLine(int x0, int y0, int x1, int y1, uint32_t color) {
    int delta_x = (x1 - x0);
    int delta_y = (y1 - y0);

    int longest_side_length = (abs(delta_x) >= abs(delta_y)) ? abs(delta_x) : abs(delta_y);

    float x_inc = delta_x / (float)longest_side_length;
    float y_inc = delta_y / (float)longest_side_length;

    float current_x = x0;
    float current_y = y0;

    for (int i = 0; i <= longest_side_length; i++) {
        drawPixel(round(current_x), round(current_y), color);
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
//
//        (x0,y0)
//          / \
//         /   \
//        /     \
//       /       \
//      /         \
//  (x1,y1)------(x2,y2)
void Renderer::rasterizeFlatBottomTriangle(int x0, int y0, int x1, int y1, int x2,
                                           int y2, uint32_t color) {
    // Find the two slopes (two triangle legs)
    float inv_slope_1 = (float)(x1 - x0) / (y1 - y0);
    float inv_slope_2 = (float)(x2 - x0) / (y2 - y0);

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
//
//  (x0,y0)------(x1,y1)
//      \         /
//       \       /
//        \     /
//         \   /
//          \ /
//        (x2,y2)
void Renderer::rasterizeFlatTopTriangle(int x0, int y0, int x1, int y1, int x2, int y2,
                                        uint32_t color) {
    // Find the two slopes (two triangle legs)
    float inv_slope_1 = (float)(x2 - x0) / (y2 - y0);
    float inv_slope_2 = (float)(x2 - x1) / (y2 - y1);

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
    auto [x0, y0, _]    = static_cast<std::tuple<int, int ,int>>(tri.points[0].get());
    auto [x1, y1, __]   = static_cast<std::tuple<int, int ,int>>(tri.points[1].get());
    auto [x2, y2, ___]  = static_cast<std::tuple<int, int ,int>>(tri.points[2].get());

    // We need to sort the vertices by y-coordinate ascending (y0 < y1 < y2)
    if (y0 > y1) {
        std::swap(y0, y1);
        std::swap(x0, x1);
    }
    if (y1 > y2) {
        std::swap(y1, y2);
        std::swap(x1, x2);
    }
    if (y0 > y1) {
        std::swap(y0, y1);
        std::swap(x0, x1);
    }

    if (y1 == y2) {
        // Draw flat-bottom triangle
        rasterizeFlatBottomTriangle(x0, y0, x1, y1, x2, y2, color);
    } else if (y0 == y1) {
        // Draw flat-top triangle
        rasterizeFlatTopTriangle(x0, y0, x1, y1, x2, y2, color);
    } else {
        // Calculate the new vertex (Mx,My) using triangle similarity
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

void Renderer::process_input() {
    SDL_Event event;
    SDL_PollEvent(&event);
    switch (event.type) {
        case SDL_QUIT:
            _isRunning = false;
            break;

        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE)
                _isRunning = false;
            if (event.key.keysym.sym == SDLK_SPACE)
                _pause = !_pause;

            if (event.key.keysym.sym == SDLK_c && _enableFaceCulling == false)
                _enableFaceCulling = true;

            if (event.key.keysym.sym == SDLK_d && _enableFaceCulling == true)
                _enableFaceCulling = false;

            if (event.key.keysym.sym == SDLK_1) {
                _wireframeModel = true;
                _VerticesModel = true;
                _raterizeModel = false;
            }

            if (event.key.keysym.sym == SDLK_2) {
                _wireframeModel = true;
                _VerticesModel = false;
                _raterizeModel = false;
            }

            if (event.key.keysym.sym == SDLK_3) {
                _wireframeModel = false;
                _VerticesModel = false;
                _raterizeModel = true;
            }

            if (event.key.keysym.sym == SDLK_4) {
                _wireframeModel = true;
                _VerticesModel = false;
                _raterizeModel = true;
            }
            break;

        default:
            break;
    }
}

bool Renderer::getWindowState() {
    return _isRunning;
}

::vector<float, 2> Renderer::project(::vector<float, 3>& point) {
    // this projection is perspective projection if you want isometric do not divide by the z-component
    return {(_fovFactor * point.x()) / point.z(), (_fovFactor * point.y()) / point.z()};
}

void Renderer::update() {
    _timer.startWatch(__func__);
    if (_firstFrame) {
        _previousFrameTime = SDL_GetTicks();
        _firstFrame = false;
    } else {
        uint32_t delay_time = _frameTargetTime - (SDL_GetTicks() - _previousFrameTime);
        if (delay_time > 0 && delay_time <= _frameTargetTime)
            SDL_Delay(delay_time);
        _previousFrameTime = SDL_GetTicks();
    }
    if (!_pause) {
        _rotation.x() += 0.01;
        _rotation.y() += 0.01;
        _rotation.z() += 0.01;

        for (const auto& face : _mesh.faces) {
            int i{0};
            std::array<::vector<float, 3>, 3> face_vertices;
            face_vertices[0] = _mesh.vertices[face.a - 1];
            face_vertices[1] = _mesh.vertices[face.b - 1];
            face_vertices[2] = _mesh.vertices[face.c - 1];
            
            for (auto& vertex : face_vertices) {
                vertex.rotateAroundX(_rotation.x());
                vertex.rotateAroundY(_rotation.y());
                vertex.rotateAroundZ(_rotation.z());
                vertex.z() += 5;
            }

            // face CUlling Test
            if(_enableFaceCulling){
                auto vec_a = face_vertices[0];
                auto vec_b = face_vertices[1];
                auto vec_c = face_vertices[2];

                // step1 calculate ab vector and ac vector
                auto vec_ab = vec_b - vec_a;
                auto vec_ac = vec_c - vec_a;

                // step2 calculate the normal
                auto vec_face_normal = vec_ab ^ vec_ac;
                vec_face_normal.normalize();

                //step3 find the camera ray (vector between camera origin and a point in the triangle)
                auto vec_camera_ray = _cameraPosition - vec_a;

                //ste4 check how aligned the camera ray with face normal
                if (vec_face_normal * vec_camera_ray < 0.0f)
                    continue;
            }

            // loop over face vertecies to perform projection
            Triangle projected_triangle;
            for(auto& vertex : face_vertices){
                auto projected_point = project(vertex);
                projected_point.x() += _width / 2; // translate
                projected_point.y() += _height / 2; // translate
                projected_triangle.points[i++] = projected_point;
            }
            _trianglesToRender.push_back(projected_triangle);
            _lastTrianglesToRender = _trianglesToRender;
        }
    }
    _timer.endWatch();
}

void Renderer::render(double timer_value) {
    _timer.startWatch(__func__);
    SDL_SetRenderDrawColor(_rendererPtr.get(), 0, 0, 0, 255);
    SDL_RenderClear(_rendererPtr.get());

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
    const ::vector<int, 2> dims1{100, 30};
    drawText(std::string(fmt::format("fps: {}", timer_value_)), dims1,
             {(_width - dims1.x()) / 2, 40});

    const ::vector<int, 2> dims2{40, 30};
    drawText("Profiles: ", {100, 30}, { 40, 40 });
    drawText(" #1 ", dims2, {40, 70});
    drawText(" #2 ", dims2, {40, 100});
    drawText(" #3 ", dims2, {40, 130});
    drawText(" #4 ", dims2, {40, 160});
    drawText("C_Key: Culling.", {150, 30}, {40, 220});
    drawText("D_Key: Disable Culling.", {200, 30}, {40, 250});
    drawText("Space_Key: Pause.", {200, 30}, {40, 280});

    SDL_RenderPresent(_rendererPtr.get());
    _trianglesToRender.clear();
    _timer.endWatch();
}

void Renderer::loadObjFileData(const std::string& obj_file_path) {
    _timer.startWatch(__func__);
    FILE* file = fopen(obj_file_path.c_str(), "r");
    if (!file) {
        std::cerr << "Error opening file: " << obj_file_path << '\n';
        return;
    }
    char line[1024];

    while (fgets(line, 1024, file)) {
        ::vector<float, 3> vertex;
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
    _timer.endWatch();
}

void Renderer::destroyWindow() {
    // SDL_Quit();
}

void Renderer::drawText(std::string_view text,
                        const ::vector<int, 2>& dims, const ::vector<int, 2>& pos) {
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