#include "GraphicsEngine.h"
#include <iostream>
#include <signal.h>

// Global engine instance for signal handling
gfx::GraphicsEngine* g_engine = nullptr;

void signalHandler(int signal) {
    if (g_engine) {
        std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
        g_engine->shutdown();
    }
}

int main(int argc, char* argv[]) {
    std::cout << "Starting Graphics Engine..." << std::endl;
    
    // Create engine instance
    gfx::GraphicsEngine engine;
    g_engine = &engine;
    
    // Setup signal handlers for graceful shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // Run the engine
    engine.run();
    
    return 0;
}
