// STL
#include <iostream>
#include <memory>
// Internal
#include "renderer.hpp"
// PROFILER
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
            renderer.setupWindow(argv[1]);
            // Game Loop
            while (renderer.getWindowState()) {
#ifdef TRACY_ENABLE
                ZoneScoped;
#endif
                timer.startWatch(__func__);
                renderer.process_input();
                renderer.update();
                renderer.render(timer.getFPS());
                timer.endWatch();
            }
            renderer.destroyWindow();
        }
    }

    return 0;
}
