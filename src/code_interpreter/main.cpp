#include "CodeInterpreter.h"
#include <iostream>
#include <signal.h>

// Global interpreter instance for signal handling
gfx::CodeInterpreter* g_interpreter = nullptr;

void signalHandler(int signal) {
    if (g_interpreter) {
        std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
        g_interpreter->shutdown();
    }
}

int main(int argc, char* argv[]) {
    std::cout << "Starting Code Interpreter..." << std::endl;
    
    // Create interpreter instance
    gfx::CodeInterpreter interpreter;
    g_interpreter = &interpreter;
    
    // Setup signal handlers for graceful shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // Run the interpreter
    interpreter.run();
    
    return 0;
}
