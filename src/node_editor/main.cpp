#include "NodeEditor.h"
#include <iostream>
#include <signal.h>

// Global editor instance for signal handling
gfx::NodeEditor* g_editor = nullptr;

void signalHandler(int signal) {
    if (g_editor) {
        std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
        g_editor->shutdown();
    }
}

int main(int argc, char* argv[]) {
    std::cout << "Starting Node Editor..." << std::endl;
    
    // Create editor instance
    gfx::NodeEditor editor;
    g_editor = &editor;
    
    // Setup signal handlers for graceful shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // Run the editor
    editor.run();
    
    return 0;
}
