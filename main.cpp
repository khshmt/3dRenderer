// STL
#include <iostream>
#include <memory>
// Internal
#include <renderer.hpp>

int main(int argc, char* argv[]) {
    {
        Renderer renderer;
        if (renderer.initializeWindow(true)) {
            renderer.setupWindow();
            // Game Loop
            while (renderer.getWindowState()) {
                // renderer.process_input();
                renderer.update();
                renderer.render();
            }
            renderer.destroyWindow();
        }
    }

    return 0;
}
