// STL
#include <iostream>
#include <memory>
// Internal
#include "renderer.hpp"
#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif

int main(int argc, char* argv[]) {
    if (!argv[1]) {
        std::cerr << "Enter a path to .obj file.\n";
        return 1;
    }
    {
        Timer timer;
        Renderer renderer;
        if (renderer.initializeWindow(true)) {
            if (renderer.setupWindow(argv[1])) {
                // Game Loop
                while (renderer.getWindowState()) {
                    FrameMarkStart("game loop");
                    timer.startWatch(__func__);
                    renderer.process_input();
                    renderer.update();
                    renderer.render(timer.getFPS());
                    timer.endWatch();
                    FrameMarkEnd("game loop");
                }
            } else {
                std::cerr << "Failed to setup SDL window.\n";
                return 1;
            }
            renderer.destroyWindow();
        } else {
            std::cerr << "Failed to initialize SDL window.\n";
            return 1;
        }
    }

    return 0;
}
