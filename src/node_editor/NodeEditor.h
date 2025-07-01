#pragma once

#include "../osc/OSCServer.h"
#include "../osc/OSCClient.h"
#include "../osc/OSCMessages.h"
#include "../core/NodeGraph.h"
#include <memory>
#include <atomic>

// Forward declarations for ImGui and GLFW
struct GLFWwindow;
struct ImGuiContext;

namespace gfx {

/**
 * @brief Node Editor with ImGui-based visual interface
 * 
 * Provides a GUI for creating and editing node graphs, communicating
 * with the Graphics Engine via OSC messages.
 */
class NodeEditor {
public:
    NodeEditor();
    ~NodeEditor();
    
    /**
     * @brief Initialize node editor with ImGui window
     * @param width Window width in pixels
     * @param height Window height in pixels  
     * @param title Window title
     * @return true if initialization successful, false otherwise
     */
    bool initialize(int width = 1200, int height = 800, const std::string& title = "Node Editor");
    
    /**
     * @brief Main GUI loop with rendering and message processing
     */
    void run();
    
    /**
     * @brief Shutdown node editor and cleanup resources
     */
    void shutdown();
    
    // OSC message handlers
    void handleEngineStatus(lo_message msg);
    void handleNodeCreated(lo_message msg);
    void handleNodeDeleted(lo_message msg);
    void handleConnectionCreated(lo_message msg);
    void handleConnectionDeleted(lo_message msg);
    void handleParameterUpdated(lo_message msg);
    void handleQuit(lo_message msg);
    void handlePing(lo_message msg);
    
    // UI operations
    void createNodeInEngine(const std::string& name, const std::string& type, float x, float y);
    void deleteNodeInEngine(int node_id);
    void connectNodesInEngine(int source_id, const std::string& source_output,
                             int target_id, const std::string& target_input);
    void disconnectNodesInEngine(int connection_id);
    void updateParameterInEngine(int node_id, const std::string& param_name, 
                               const std::string& value);
    
    // Graph operations
    void saveGraph(const std::string& filename);
    void loadGraph(const std::string& filename);
    
    // Status
    bool isRunning() const { return running_; }
    
private:
    /**
     * @brief Setup OSC message handlers
     */
    void setupOSCHandlers();
    
    /**
     * @brief Render ImGui interface
     */
    void renderUI();
    
    /**
     * @brief Render node graph editor
     */
    void renderNodeGraph();
    
    /**
     * @brief Render node creation menu
     */
    void renderNodeCreationMenu();
    
    /**
     * @brief Render properties panel for selected node
     */
    void renderPropertiesPanel();
    
    /**
     * @brief Initialize ImGui context and window
     */
    bool initializeImGui(int width, int height, const std::string& title);
    
    /**
     * @brief Shutdown ImGui and cleanup
     */
    void shutdownImGui();
    
    /**
     * @brief GLFW window callbacks
     */
    static void glfwErrorCallback(int error, const char* description);
    static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    
    GLFWwindow* window_;                                ///< GLFW window handle
    ImGuiContext* imgui_context_;                       ///< ImGui context
    
    std::unique_ptr<OSCServer> osc_server_;             ///< OSC server for communication
    std::unique_ptr<OSCClient> engine_client_;          ///< OSC client for graphics engine
    std::unique_ptr<OSCClient> code_interpreter_client_; ///< OSC client for code interpreter
    
    std::unique_ptr<NodeGraph> local_graph_;            ///< Local copy of node graph for UI
    
    std::atomic<bool> running_;                         ///< Main loop running state
    bool engine_connected_;                             ///< Connection status to graphics engine
    
    // UI state
    int selected_node_id_;                              ///< Currently selected node ID
    bool show_node_creation_menu_;                      ///< Show node creation context menu
    float menu_position_x_, menu_position_y_;           ///< Context menu position
    
    // Next node ID for creation
    int next_node_id_;                                  ///< Counter for new node IDs
    
    // Window properties
    int window_width_;                                  ///< Current window width
    int window_height_;                                 ///< Current window height
};

} // namespace gfx
