// STL
#include <iostream>
#include <memory>
// Internal
#include <renderer.hpp>

int main(int argc, char* argv[]) {
    math::Vector<int,3> vec1 = {1,2,4};
    math::Vector<int,3> vec2 = {1,-2,2};
    auto vec = vec1*vec2;
    // std::cout << vec.x() << " " << vec.y() << " " << vec.z() << '\n';
    std::cout << vec << '\n';
    if (!argv[1]) {
        std::cerr << "Enter a path to .obj file.\n";
        return 1;
    }
    {
        Renderer renderer;
        if (renderer.initializeWindow(true)) {
            renderer.setupWindow(argv[1]);
            // Game Loop
            while (renderer.getWindowState()) {
                // renderer.process_input();
                renderer.update();
                renderer.render();
            }
            renderer.destroyWindow();
            std::cout << sizeof(renderer) << '\n';
        }
    }

    return 0;
}
