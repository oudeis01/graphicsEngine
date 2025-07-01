#pragma once

#include "RenderContext.h"
#include "ShaderManager.h"
#include "Pipeline.h"
#include "../osc/OSCServer.h"
#include "../osc/OSCClient.h"
#include "../osc/OSCMessages.h"
#include "../core/NodeGraph.h"
#include <memory>
#include <atomic>
#include <thread>

namespace gfx {

/**
 * @brief Main graphics engine with OpenGL rendering window and OSC server
 * 
 * Manages OpenGL context, shader compilation, rendering pipeline,
 * and OSC communication for the distributed graphics system.
 */
class GraphicsEngine {
public:
    GraphicsEngine();
    ~GraphicsEngine();
    
    /**
     * @brief Initialize graphics engine with window and OSC server
     * @param width Window width in pixels  
     * @param height Window height in pixels
     * @param title Window title
     * @return true if initialization successful, false otherwise
     */
    bool initialize(int width = 800, int height = 600, const std::string& title = "Graphics Engine");
    
    /**
     * @brief Main rendering and message processing loop
     */
    void run();
    
    /**
     * @brief Shutdown graphics engine and cleanup resources
     */
    void shutdown();
    
    // OSC message handlers
    void handleCreateNode(lo_message msg);
    void handleDeleteNode(lo_message msg);
    void handleUpdateNode(lo_message msg);
    void handleSetParameter(lo_message msg);
    void handleConnectNodes(lo_message msg);
    void handleDisconnectNodes(lo_message msg);
    void handleRenderFrame(lo_message msg);
    void handleQuit(lo_message msg);
    void handlePing(lo_message msg);
    
    // Node management
    void createNode(int id, const std::string& name, const std::string& type);
    void deleteNode(int id);
    void updateNodeParameter(int node_id, const std::string& param_name, 
                           const std::string& value);
    
    // Connection management
    void connectNodes(int source_id, const std::string& source_output,
                     int target_id, const std::string& target_input);
    void disconnectNodes(int connection_id);
    
    // Rendering
    void renderFrame();
    
    // Status
    bool isRunning() const { return running_; }

private:
    /**
     * @brief Setup OSC message handlers
     */
    void setupOSCHandlers();
    
    /**
     * @brief Process OSC messages in main loop
     */
    void processOSCMessages();
    
    /**
     * @brief Main rendering loop
     */
    void mainLoop();
    
    /**
     * @brief Rendering loop for background thread
     */
    void renderingLoop();
    
    std::unique_ptr<RenderContext> render_context_;     ///< OpenGL context and window management
    std::shared_ptr<ShaderManager> shader_manager_;     ///< LYGIA-based shader compilation
    std::unique_ptr<Pipeline> pipeline_;                ///< Rendering pipeline management
    std::unique_ptr<OSCServer> osc_server_;             ///< OSC server for external communication
    std::unique_ptr<OSCClient> node_editor_client_;     ///< OSC client for node editor communication
    std::unique_ptr<OSCClient> code_interpreter_client_; ///< OSC client for code interpreter communication
    
    std::unique_ptr<NodeGraph> node_graph_;             ///< Current node graph state
    std::atomic<bool> running_;                         ///< Main loop running state
    std::thread rendering_thread_;                      ///< Background rendering thread
    bool should_render_;                                  ///< Flag to control rendering
    float target_fps_;                                   ///< Target frames per second for rendering
    float frame_time_;                                   ///< Time per frame in seconds
    
    // Window properties
    int window_width_;                                  ///< Current window width
    int window_height_;                                 ///< Current window height
};

} // namespace gfx
