//STL
#include <iostream>
//INTERNAL
#include <renderer.hpp>

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

    window_ptr = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>(
        SDL_CreateWindow(nullptr, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _width, _height,
                         SDL_WINDOW_RESIZABLE),
        SDL_DestroyWindow);

    if (!window_ptr) {
        std::cerr << "Error creating a window\n";
        return false;
    }

    renderer_ptr = std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)>(
        SDL_CreateRenderer(window_ptr.get(), -1, 0), SDL_DestroyRenderer);
    if (!renderer_ptr) {
        std::cerr << "Error creating a renderer\n";
        return false;
    }
    if (fullscreen) {
        SDL_SetWindowFullscreen(window_ptr.get(), SDL_WINDOW_FULLSCREEN);
    }
    is_running = true;
    th = std::thread([this]() { process_input(); });
    return true;
}

void Renderer::setupWindow() {
    color_buffer.reserve(_width * _height);

    color_buffer_texture_ptr = std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>(
        SDL_CreateTexture(renderer_ptr.get(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                          _width, _height),
        SDL_DestroyTexture);
}

void Renderer::drawPixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < _width && y >= 0 && y < _height) {
        color_buffer[(_width * y) + x] = color;
    }
}

void Renderer::drawGrid() {
    for (uint32_t y{0}; y < _height; y += 20) {
        for (uint32_t x{0}; x < _width; x += 20) {
            color_buffer[(_width * y) + x] = 0xFFFFFFFF;
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

void Renderer::drawLine(float x0, float y0, float x1, float y1, uint32_t color) {
    // DDA (Digital Differential Analayzer) ALgorithm Approach
    auto delta_x = x1 - x0;
    auto delta_y = y1 - y0;

    auto side_length = fabs(delta_x) >= fabs(delta_y) ? fabs(delta_x) : fabs(delta_y);

    auto x_inc = delta_x / (float)side_length;
    auto y_inc = delta_y / (float)side_length;

    // float current_x = x0;
    // float current_y = y0;

    for (int i{0}; i < side_length; ++i) {
        drawPixel(round(x0), round(y0), color);
        x0 += x_inc;
        y0 += y_inc;
    }
}

void Renderer::renderColorBuffer() {
    SDL_UpdateTexture(color_buffer_texture_ptr.get(), nullptr, color_buffer.data(),
                      (int)(sizeof(uint32_t) * _width)  //Pitch ==> size of one row in bytes
    );

    SDL_RenderCopy(renderer_ptr.get(), color_buffer_texture_ptr.get(), nullptr, nullptr);
}

void Renderer::clearColorBuffer(uint32_t color) {
    for (uint32_t y{0}; y < _height; ++y) {
        for (uint32_t x{0}; x < _width; ++x) {
            color_buffer[(_width * y) + x] = color;
        }
    }
}

void Renderer::process_input() {
    while (is_running) {
        SDL_Event event;
        SDL_PollEvent(&event);
        switch (event.type) {
            case SDL_QUIT:
                is_running = false;
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    is_running = false;
                if (event.key.keysym.sym == SDLK_SPACE && pause == false)
                    pause = true;
                else
                    pause = false;

                if (event.key.keysym.sym == SDLK_UP && up == false)
                    up = true;
                else
                    up = false;

                if (event.key.keysym.sym == SDLK_LEFT && left == false)
                    left = true;
                else
                    left = false;
                break;
            default:
                break;
        }
    }
}

bool Renderer::getWindowState() {
    return is_running;
}

math::Vector<float, 2> Renderer::project(math::Vector<float, 3> point) {
    // this projection is perspective projection if you want isometric do not divide by the z-component
    return {(fov_factor * point.x()) / point.z(), (fov_factor * point.y()) / point.z()};
}

void Renderer::update() {
    if (firstFrame) {
        previousFrameTime = SDL_GetTicks();
        firstFrame = false;
    } else {
        uint32_t delay_time = frameTargetTime - (SDL_GetTicks() - previousFrameTime);
        if (delay_time > 0 && delay_time <= frameTargetTime)
            ;
        SDL_Delay(delay_time);
        previousFrameTime = SDL_GetTicks();
    }
    if (!pause) {
        cube_rotation.x() += 0.01;
        cube_rotation.y() += 0.01;
        cube_rotation.z() += 0.01;

        int j{0};
        for (auto face : mesh_faces) {
            int i{0};
            std::array<math::Vector<float, 3>, 3> face_vertices;
            face_vertices[0] = mesh_vertices[face.a - 1];
            face_vertices[1] = mesh_vertices[face.b - 1];
            face_vertices[2] = mesh_vertices[face.c - 1];
            Triangle projected_triangle;
            for (auto vertex : face_vertices) {
                auto transformed_vertex = vertex;
                transformed_vertex.rotateAroundX(cube_rotation.x());
                transformed_vertex.rotateAroundY(cube_rotation.y());
                transformed_vertex.rotateAroundZ(cube_rotation.z());

                transformed_vertex.z() -= camera_position.z();
                auto projected_point = project(transformed_vertex);
                projected_point.x() += _width / 2;
                projected_point.y() += _height / 2;
                projected_triangle.points[i++] = projected_point;
            }
            triangles_to_render[j++] = projected_triangle;
        }
    }
}

void Renderer::render() {
    SDL_SetRenderDrawColor(renderer_ptr.get(), 0, 0, 0, 255);
    SDL_RenderClear(renderer_ptr.get());

    clearColorBuffer(0xFF000000);

    drawGrid();
    for (auto triangle : triangles_to_render) {
        drawRect(triangle.points[0].x(), triangle.points[0].y(), 3, 3, 0xFFFFFF00);
        drawRect(triangle.points[1].x(), triangle.points[1].y(), 3, 3, 0xFFFFFF00);
        drawRect(triangle.points[2].x(), triangle.points[2].y(), 3, 3, 0xFFFFFF00);

        drawLine(triangle.points[0].x(), triangle.points[0].y(), triangle.points[1].x(),
                 triangle.points[1].y(), 0xFF00FF00);
        drawLine(triangle.points[1].x(), triangle.points[1].y(), triangle.points[2].x(),
                 triangle.points[2].y(), 0xFF00FF00);
        drawLine(triangle.points[2].x(), triangle.points[2].y(), triangle.points[0].x(),
                 triangle.points[0].y(), 0xFF00FF00);
    }

    renderColorBuffer();
    SDL_RenderPresent(renderer_ptr.get());
}

void Renderer::destroyWindow() {
    if (th.joinable()) {
        th.join();
    }
    // SDL_Quit();
}