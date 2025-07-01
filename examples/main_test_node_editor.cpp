#include "graphics/GraphicsEngine.h"
#include <iostream>
#include <exception>

int main() {
    try {
        // Create graphics engine
        GraphicsEngine engine;

        // Initialize with window
        if (!engine.initialize(800, 600, "Graphics Engine - Node Editor Test")) {
            std::cerr << "Failed to initialize graphics engine" << std::endl;
            return -1;
        }

        // Enable hot-reloading
        engine.enableHotReload(true);
        
        // Node Editor opens automatically in separate window
        
        // Set shader reload callback
        engine.setShaderReloadCallback([]() {
            std::cout << "Shader hot-reload completed!" << std::endl;
        });

        // Run the engine
        std::cout << "Starting graphics engine..." << std::endl;
        std::cout << "Node Editor should be automatically opened for testing" << std::endl;
        std::cout << "Press F1 to toggle Node Editor, or close window to exit" << std::endl;
        engine.run();

        std::cout << "Graphics engine shut down normally." << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return -1;
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        return -1;
    }
}
