#include "GraphicsEngine.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <lo/lo.h>

namespace gfx {

GraphicsEngine::GraphicsEngine() 
    : running_(false), should_render_(true), target_fps_(60.0f), frame_time_(1.0f/60.0f),
      window_width_(800), window_height_(600) {
    
    osc_server_ = std::make_unique<OSCServer>(osc::ENGINE_PORT);
    node_editor_client_ = std::make_unique<OSCClient>();
    code_interpreter_client_ = std::make_unique<OSCClient>();
    node_graph_ = std::make_unique<NodeGraph>();
}

GraphicsEngine::~GraphicsEngine() {
    shutdown();
}

bool GraphicsEngine::initialize(int width, int height, const std::string& title) {
    std::cout << "Initializing Graphics Engine..." << std::endl;
    
    window_width_ = width;
    window_height_ = height;
    
    // Initialize rendering context and window
    render_context_ = std::make_unique<RenderContext>();
    if (!render_context_->initialize(width, height, title)) {
        std::cerr << "Failed to initialize RenderContext" << std::endl;
        return false;
    }
    
    // Initialize shader manager
    shader_manager_ = std::make_shared<ShaderManager>();
#ifdef LYGIA_PATH
    if (!shader_manager_->initialize(LYGIA_PATH)) {
#else
    if (!shader_manager_->initialize("../external/lygia")) {
#endif
        std::cerr << "Failed to initialize ShaderManager" << std::endl;
        return false;
    }
    
    // Initialize pipeline
    pipeline_ = std::make_unique<Pipeline>();
    if (!pipeline_->initialize(shader_manager_)) {
        std::cerr << "Failed to initialize Pipeline" << std::endl;
        return false;
    }
    
    // Start OSC server
    if (!osc_server_->start()) {
        std::cerr << "Failed to start OSC server" << std::endl;
        return false;
    }
    
    // Setup OSC message handlers
    setupOSCHandlers();
    
    // Connect to other components (they might not be running yet, that's ok)
    node_editor_client_->connect("localhost", osc::NODE_EDITOR_PORT);
    code_interpreter_client_->connect("localhost", osc::CODE_INTERPRETER_PORT);
    
    std::cout << "Graphics Engine initialized successfully" << std::endl;
    std::cout << "OSC Server listening on port " << osc::ENGINE_PORT << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    
    return true;
}

void GraphicsEngine::run() {
    if (!initialize(window_width_, window_height_, "Graphics Engine")) {
        return;
    }
    
    running_ = true;
    
    // Send initial status to other components
    node_editor_client_->sendMessage(std::string(osc::engine::STATUS), std::string("running"));
    code_interpreter_client_->sendMessage(std::string(osc::engine::STATUS), std::string("running"));
    
    std::cout << "Graphics Engine is running. Close the window or press Ctrl+C to quit." << std::endl;
    
    // Simple main loop instead of separate rendering thread for now
    auto last_frame_time = std::chrono::high_resolution_clock::now();
    
    while (running_ && render_context_ && !render_context_->shouldClose()) {
        auto current_time = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
            current_time - last_frame_time).count();
        
        // Poll window events
        render_context_->pollEvents();
        
        if (elapsed >= frame_time_ && should_render_) {
            renderFrame();
            last_frame_time = current_time;
        } else {
            // Sleep for a short time to avoid busy waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    
    // If window was closed, trigger shutdown
    if (render_context_ && render_context_->shouldClose()) {
        running_ = false;
    }
}

void GraphicsEngine::shutdown() {
    if (!running_) {
        return;
    }
    
    std::cout << "Shutting down Graphics Engine..." << std::endl;
    
    running_ = false;
    
    // Notify other components
    node_editor_client_->sendMessage(std::string(osc::engine::STATUS), std::string("shutting_down"));
    code_interpreter_client_->sendMessage(std::string(osc::engine::STATUS), std::string("shutting_down"));
    
    // Stop OSC server
    if (osc_server_) {
        osc_server_->stop();
    }
    
    // Disconnect clients
    if (node_editor_client_) {
        node_editor_client_->disconnect();
    }
    if (code_interpreter_client_) {
        code_interpreter_client_->disconnect();
    }
    
    // Clean up rendering resources
    pipeline_.reset();
    shader_manager_.reset();
    if (render_context_) {
        render_context_->shutdown();
        render_context_.reset();
    }
    
    std::cout << "Graphics Engine shutdown complete" << std::endl;
}

void GraphicsEngine::setupOSCHandlers() {
    // Node management
    osc_server_->addHandler(osc::engine::CREATE_NODE, 
        [this](const std::string& path, lo_message msg) { handleCreateNode(msg); });
    
    osc_server_->addHandler(osc::engine::DELETE_NODE,
        [this](const std::string& path, lo_message msg) { handleDeleteNode(msg); });
    
    osc_server_->addHandler(osc::engine::UPDATE_NODE,
        [this](const std::string& path, lo_message msg) { handleUpdateNode(msg); });
    
    osc_server_->addHandler(osc::engine::SET_PARAMETER,
        [this](const std::string& path, lo_message msg) { handleSetParameter(msg); });
    
    // Connection management
    osc_server_->addHandler(osc::engine::CONNECT_NODES,
        [this](const std::string& path, lo_message msg) { handleConnectNodes(msg); });
    
    osc_server_->addHandler(osc::engine::DISCONNECT_NODES,
        [this](const std::string& path, lo_message msg) { handleDisconnectNodes(msg); });
    
    // Rendering
    osc_server_->addHandler(osc::engine::RENDER_FRAME,
        [this](const std::string& path, lo_message msg) { handleRenderFrame(msg); });
    
    // Control
    osc_server_->addHandler(osc::engine::QUIT,
        [this](const std::string& path, lo_message msg) { handleQuit(msg); });
    
    osc_server_->addHandler(osc::common::PING,
        [this](const std::string& path, lo_message msg) { handlePing(msg); });
}

void GraphicsEngine::handleCreateNode(lo_message msg) {
    if (lo_message_get_argc(msg) >= 3) {
        int id = lo_message_get_argv(msg)[0]->i;
        const char* name = &lo_message_get_argv(msg)[1]->s;
        const char* type = &lo_message_get_argv(msg)[2]->s;
        
        createNode(id, name, type);
        std::cout << "Created node: " << id << " (" << name << ", " << type << ")" << std::endl;
        
        // Notify other components
        std::string message = std::to_string(id) + "," + std::string(name) + "," + std::string(type);
        node_editor_client_->sendMessage(std::string("/engine/node/created"), message);
    }
}

void GraphicsEngine::handleDeleteNode(lo_message msg) {
    if (lo_message_get_argc(msg) >= 1) {
        int id = lo_message_get_argv(msg)[0]->i;
        
        deleteNode(id);
        std::cout << "Deleted node: " << id << std::endl;
        
        // Notify other components
        node_editor_client_->sendMessage(std::string("/engine/node/deleted"), std::to_string(id));
    }
}

void GraphicsEngine::handleUpdateNode(lo_message msg) {
    // TODO: Implement node update logic
    std::cout << "Handle update node (not implemented)" << std::endl;
}

void GraphicsEngine::handleSetParameter(lo_message msg) {
    if (lo_message_get_argc(msg) >= 3) {
        int node_id = lo_message_get_argv(msg)[0]->i;
        const char* param_name = &lo_message_get_argv(msg)[1]->s;
        const char* value = &lo_message_get_argv(msg)[2]->s;
        
        updateNodeParameter(node_id, param_name, value);
        std::cout << "Updated parameter: node " << node_id << ", " << param_name << " = " << value << std::endl;
        
        // Notify other components
        std::string message = std::to_string(node_id) + "," + std::string(param_name) + "," + std::string(value);
        node_editor_client_->sendMessage(std::string("/engine/parameter/updated"), message);
    }
}

void GraphicsEngine::handleConnectNodes(lo_message msg) {
    if (lo_message_get_argc(msg) >= 4) {
        int source_id = lo_message_get_argv(msg)[0]->i;
        const char* source_output = &lo_message_get_argv(msg)[1]->s;
        int target_id = lo_message_get_argv(msg)[2]->i;
        const char* target_input = &lo_message_get_argv(msg)[3]->s;
        
        connectNodes(source_id, source_output, target_id, target_input);
        std::cout << "Connected nodes: " << source_id << "." << source_output 
                  << " -> " << target_id << "." << target_input << std::endl;
        
        // Notify other components
        std::string message = std::to_string(source_id) + "," + std::string(source_output) + "," + 
                             std::to_string(target_id) + "," + std::string(target_input);
        node_editor_client_->sendMessage(std::string("/engine/connection/created"), message);
    }
}

void GraphicsEngine::handleDisconnectNodes(lo_message msg) {
    if (lo_message_get_argc(msg) >= 1) {
        int connection_id = lo_message_get_argv(msg)[0]->i;
        
        disconnectNodes(connection_id);
        std::cout << "Disconnected connection: " << connection_id << std::endl;
        
        // Notify other components
        node_editor_client_->sendMessage(std::string("/engine/connection/deleted"), std::to_string(connection_id));
    }
}

void GraphicsEngine::handleRenderFrame(lo_message msg) {
    renderFrame();
}

void GraphicsEngine::handleQuit(lo_message msg) {
    std::cout << "Received quit message" << std::endl;
    running_ = false;
}

void GraphicsEngine::handlePing(lo_message msg) {
    // Respond with pong
    node_editor_client_->sendMessage(std::string(osc::common::PONG));
    code_interpreter_client_->sendMessage(std::string(osc::common::PONG));
}

void GraphicsEngine::createNode(int id, const std::string& name, const std::string& type) {
    // TODO: Create actual node based on type
    // For now, just add to the graph as a placeholder
    osc::NodeType node_type = osc::stringToNodeType(type);
    
    // Create a simple node (this would be factory-based in a real implementation)
    class SimpleNode : public Node {
    public:
        SimpleNode(int id, const std::string& name, osc::NodeType type) 
            : Node(id, name, type) {}
        void process() override {
            // Simple processing placeholder
        }
    };
    
    auto node = std::make_shared<SimpleNode>(id, name, node_type);
    node_graph_->addNode(node);
}

void GraphicsEngine::deleteNode(int id) {
    node_graph_->removeNode(id);
}

void GraphicsEngine::updateNodeParameter(int node_id, const std::string& param_name, 
                                        const std::string& value) {
    auto node = node_graph_->getNode(node_id);
    if (node) {
        auto param = node->getParameter(param_name);
        if (param) {
            param->fromString(value);
        }
    }
}

void GraphicsEngine::connectNodes(int source_id, const std::string& source_output,
                                 int target_id, const std::string& target_input) {
    // Generate a connection ID
    static int next_connection_id = 1;
    auto connection = std::make_shared<Connection>(next_connection_id++, 
                                                  source_id, source_output,
                                                  target_id, target_input);
    node_graph_->addConnection(connection);
}

void GraphicsEngine::disconnectNodes(int connection_id) {
    node_graph_->removeConnection(connection_id);
}

void GraphicsEngine::renderFrame() {
    if (!render_context_ || !pipeline_) {
        return;
    }
    
    // Clear the screen
    render_context_->clear();
    
    // Process all nodes in topological order (if node graph is valid)
    if (node_graph_) {
        try {
            auto nodes = node_graph_->getTopologicalOrder();
            for (auto& node : nodes) {
                if (node) {
                    node->process();
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error processing nodes: " << e.what() << std::endl;
        }
    }
    
    // Render the pipeline with deltaTime
    try {
        pipeline_->render(frame_time_);
    } catch (const std::exception& e) {
        std::cerr << "Error rendering pipeline: " << e.what() << std::endl;
    }
    
    // Swap buffers to display the frame
    render_context_->swapBuffers();
}

void GraphicsEngine::renderingLoop() {
    auto last_frame_time = std::chrono::high_resolution_clock::now();
    
    while (running_ && render_context_ && !render_context_->shouldClose()) {
        auto current_time = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
            current_time - last_frame_time).count();
        
        // Poll window events
        render_context_->pollEvents();
        
        if (elapsed >= frame_time_ && should_render_) {
            renderFrame();
            last_frame_time = current_time;
        } else {
            // Sleep for a short time to avoid busy waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    
    // If window was closed, trigger shutdown
    if (render_context_ && render_context_->shouldClose()) {
        running_ = false;
    }
}

} // namespace gfx
