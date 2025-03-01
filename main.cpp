// STL
#include <iostream>
#include <memory>
// Internal
#include <renderer.hpp>
#include "timer.hpp"

int main(int argc, char* argv[]) {
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
                Timer timer;
                renderer.process_input();
                renderer.update();
                renderer.render();
            }
            renderer.destroyWindow();
        }
        std::cout << sizeof(renderer) << '\n';
    }

    return 0;
}
