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
                    #ifdef TRACY_ENABLE
                    FrameMarkStart("game loop");
                    #endif
                    timer.startWatch(__func__);
                    
                    renderer.processInput(); // #1-Game Loop
                    renderer.update(); // #2-Game Loop
                    renderer.render(timer.getFPS()); // #3-Game Loop
                    
                    timer.endWatch();
                    #ifdef TRACY_ENABLE
                    FrameMarkEnd("game loop");
                    #endif
                }
            } else {
                std::cerr << "Failed to setup SDL window.\n";
                return 2;
            }
            renderer.destroyWindow();
        } else {
            std::cerr << "Failed to initialize SDL window.\n";
            return 3;
        }
    }

    return 0;
}
