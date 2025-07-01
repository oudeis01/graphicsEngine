#include "NodeEditor.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <lo/lo.h>

// OpenGL includes (MUST come before GLFW and ImGui)
#include <GL/glew.h>

// GLFW includes  
#include <GLFW/glfw3.h>

// ImGui includes
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace gfx {

NodeEditor::NodeEditor() 
    : window_(nullptr), imgui_context_(nullptr), 
      running_(false), engine_connected_(false), 
      selected_node_id_(-1), show_node_creation_menu_(false),
      menu_position_x_(0.0f), menu_position_y_(0.0f), next_node_id_(1),
      window_width_(1200), window_height_(800) {
    
    osc_server_ = std::make_unique<OSCServer>(osc::NODE_EDITOR_PORT);
    engine_client_ = std::make_unique<OSCClient>();
    code_interpreter_client_ = std::make_unique<OSCClient>();
    local_graph_ = std::make_unique<NodeGraph>();
}

NodeEditor::~NodeEditor() {
    shutdown();
}

void NodeEditor::glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

void NodeEditor::glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

bool NodeEditor::initialize(int width, int height, const std::string& title) {
    std::cout << "Initializing Node Editor..." << std::endl;
    
    window_width_ = width;
    window_height_ = height;
    
    // Initialize ImGui window
    if (!initializeImGui(width, height, title)) {
        std::cerr << "Failed to initialize ImGui window" << std::endl;
        return false;
    }
    
    // Start OSC server
    if (!osc_server_->start()) {
        std::cerr << "Failed to start OSC server" << std::endl;
        return false;
    }
    
    // Setup OSC message handlers
    setupOSCHandlers();
    
    // Connect to engine and code interpreter
    if (engine_client_->connect("localhost", osc::ENGINE_PORT)) {
        engine_connected_ = true;
        std::cout << "Connected to Graphics Engine" << std::endl;
    } else {
        std::cout << "Graphics Engine not available (will retry)" << std::endl;
    }
    
    code_interpreter_client_->connect("localhost", osc::CODE_INTERPRETER_PORT);
    
    std::cout << "Node Editor initialized successfully" << std::endl;
    std::cout << "OSC Server listening on port " << osc::NODE_EDITOR_PORT << std::endl;
    
    return true;
}

bool NodeEditor::initializeImGui(int width, int height, const std::string& title) {
    // Setup GLFW error callback
    glfwSetErrorCallback(glfwErrorCallback);
    
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    
    // Create window with graphics context
    window_ = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (window_ == nullptr) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1); // Enable vsync
    
    // Setup key callback
    glfwSetKeyCallback(window_, glfwKeyCallback);
    
    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwDestroyWindow(window_);
        glfwTerminate();
        return false;
    }
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    imgui_context_ = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    
    return true;
}

void NodeEditor::run() {
    if (!initialize()) {
        return;
    }
    
    running_ = true;
    
    // Send initial status to other components
    if (engine_connected_) {
        engine_client_->sendMessage(std::string(osc::node_editor::STATUS), std::string("running"));
    }
    code_interpreter_client_->sendMessage(std::string(osc::node_editor::STATUS), std::string("running"));
    
    std::cout << "Node Editor is running. Close window or press ESC to quit." << std::endl;
    
    // Main loop
    while (running_ && !glfwWindowShouldClose(window_)) {
        // Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();
        
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // Render UI
        renderUI();
        
        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window_, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.15f, 0.15f, 0.15f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window_);
        
        // Small sleep to prevent excessive CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    }
}

void NodeEditor::renderUI() {
    // Main menu bar
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Graph")) {
                local_graph_ = std::make_unique<NodeGraph>();
            }
            if (ImGui::MenuItem("Save Graph")) {
                saveGraph("graph.json");
            }
            if (ImGui::MenuItem("Load Graph")) {
                loadGraph("graph.json");
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Quit")) {
                running_ = false;
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Engine")) {
            if (ImGui::MenuItem("Reconnect")) {
                if (engine_client_->connect("localhost", osc::ENGINE_PORT)) {
                    engine_connected_ = true;
                    std::cout << "Reconnected to Graphics Engine" << std::endl;
                }
            }
            ImGui::Separator();
            ImGui::Text("Status: %s", engine_connected_ ? "Connected" : "Disconnected");
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }
    
    // Render node graph editor
    renderNodeGraph();
    
    // Render properties panel
    renderPropertiesPanel();
}

void NodeEditor::renderNodeGraph() {
    ImGui::Begin("Node Graph");
    
    // Get available space for the canvas
    ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
    ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
    
    // Ensure minimum canvas size to avoid ImGui assertion
    if (canvas_sz.x <= 0.0f) canvas_sz.x = 100.0f;
    if (canvas_sz.y <= 0.0f) canvas_sz.y = 100.0f;
    
    ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);
    
    // Draw background
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
    draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));
    
    // This will catch our interactions
    ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
    const bool is_hovered = ImGui::IsItemHovered(); // Hovered
    const bool is_active = ImGui::IsItemActive();   // Held
    const ImVec2 origin(canvas_p0.x, canvas_p0.y); // Lock scrolled origin
    const ImVec2 mouse_pos_in_canvas(ImGui::GetIO().MousePos.x - origin.x, ImGui::GetIO().MousePos.y - origin.y);
    
    // Context menu (under default mouse threshold)
    ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
    if (drag_delta.x == 0.0f && drag_delta.y == 0.0f) {
        ImGui::OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);
    }
    
    if (ImGui::BeginPopup("context")) {
        if (ImGui::MenuItem("Create Output Node")) {
            createNodeInEngine("Output", "output", mouse_pos_in_canvas.x, mouse_pos_in_canvas.y);
        }
        if (ImGui::MenuItem("Create Texture Node")) {
            createNodeInEngine("Texture", "texture", mouse_pos_in_canvas.x, mouse_pos_in_canvas.y);
        }
        if (ImGui::MenuItem("Create Math Node")) {
            createNodeInEngine("Math", "math", mouse_pos_in_canvas.x, mouse_pos_in_canvas.y);
        }
        if (ImGui::MenuItem("Create Color Node")) {
            createNodeInEngine("Color", "color", mouse_pos_in_canvas.x, mouse_pos_in_canvas.y);
        }
        ImGui::EndPopup();
    }
    
    // Simple representation of nodes (this would be replaced with actual node editor)
    auto& nodes = local_graph_->getNodes();
    for (auto& node_pair : nodes) {
        auto& node = node_pair.second;
        
        // Simple node representation
        ImVec2 node_pos(origin.x + 50 + node->getId() * 150, origin.y + 50);
        ImVec2 node_size(120, 60);
        
        // Node background
        ImU32 node_color = (selected_node_id_ == node->getId()) ? IM_COL32(100, 100, 200, 255) : IM_COL32(80, 80, 80, 255);
        draw_list->AddRectFilled(node_pos, ImVec2(node_pos.x + node_size.x, node_pos.y + node_size.y), node_color);
        draw_list->AddRect(node_pos, ImVec2(node_pos.x + node_size.x, node_pos.y + node_size.y), IM_COL32(255, 255, 255, 255));
        
        // Node label
        std::string label = node->getName() + " (" + std::to_string(node->getId()) + ")";
        draw_list->AddText(ImVec2(node_pos.x + 5, node_pos.y + 5), IM_COL32(255, 255, 255, 255), label.c_str());
        
        // Check if node is clicked
        if (is_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            if (mouse_pos_in_canvas.x >= node_pos.x - origin.x && mouse_pos_in_canvas.x <= node_pos.x - origin.x + node_size.x &&
                mouse_pos_in_canvas.y >= node_pos.y - origin.y && mouse_pos_in_canvas.y <= node_pos.y - origin.y + node_size.y) {
                selected_node_id_ = node->getId();
            }
        }
    }
    
    ImGui::End();
}

void NodeEditor::renderPropertiesPanel() {
    ImGui::Begin("Properties");
    
    if (selected_node_id_ >= 0) {
        auto node = local_graph_->getNode(selected_node_id_);
        if (node) {
            ImGui::Text("Node: %s", node->getName().c_str());
            ImGui::Text("ID: %d", node->getId());
            ImGui::Text("Type: %s", osc::nodeTypeToString(node->getType()).c_str());
            
            ImGui::Separator();
            
            // Show parameters
            auto& parameters = node->getParameters();
            for (auto& param_pair : parameters) {
                auto& param = param_pair.second;
                ImGui::Text("%s: %s", param->getName().c_str(), param->toString().c_str());
            }
            
            ImGui::Separator();
            
            if (ImGui::Button("Delete Node")) {
                deleteNodeInEngine(selected_node_id_);
                selected_node_id_ = -1;
            }
        }
    } else {
        ImGui::Text("No node selected");
        ImGui::Text("Right-click in the graph to create nodes");
    }
    
    ImGui::End();
}

void NodeEditor::renderNodeCreationMenu() {
    // This is handled in renderNodeGraph() via context menu
}

void NodeEditor::shutdown() {
    if (!running_) {
        return;
    }
    
    std::cout << "Shutting down Node Editor..." << std::endl;
    
    running_ = false;
    
    // Notify other components
    if (engine_connected_) {
        engine_client_->sendMessage(std::string(osc::node_editor::STATUS), std::string("shutting_down"));
    }
    code_interpreter_client_->sendMessage(std::string(osc::node_editor::STATUS), std::string("shutting_down"));
    
    // Shutdown ImGui
    shutdownImGui();
    
    // Stop OSC server
    if (osc_server_) {
        osc_server_->stop();
    }
    
    // Disconnect clients
    if (engine_client_) {
        engine_client_->disconnect();
    }
    if (code_interpreter_client_) {
        code_interpreter_client_->disconnect();
    }
    
    std::cout << "Node Editor shutdown complete" << std::endl;
}

void NodeEditor::shutdownImGui() {
    if (imgui_context_) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext(imgui_context_);
        imgui_context_ = nullptr;
    }
    
    if (window_) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }
    
    glfwTerminate();
}

void NodeEditor::setupOSCHandlers() {
    // Engine status updates
    osc_server_->addHandler(osc::engine::STATUS,
        [this](const std::string& path, lo_message msg) { handleEngineStatus(msg); });
    
    // Node lifecycle
    osc_server_->addHandler("/engine/node/created",
        [this](const std::string& path, lo_message msg) { handleNodeCreated(msg); });
    
    osc_server_->addHandler("/engine/node/deleted",
        [this](const std::string& path, lo_message msg) { handleNodeDeleted(msg); });
    
    // Connections
    osc_server_->addHandler("/engine/connection/created",
        [this](const std::string& path, lo_message msg) { handleConnectionCreated(msg); });
    
    osc_server_->addHandler("/engine/connection/deleted",
        [this](const std::string& path, lo_message msg) { handleConnectionDeleted(msg); });
    
    // Parameters
    osc_server_->addHandler("/engine/parameter/updated",
        [this](const std::string& path, lo_message msg) { handleParameterUpdated(msg); });
    
    // Control
    osc_server_->addHandler(osc::node_editor::QUIT,
        [this](const std::string& path, lo_message msg) { handleQuit(msg); });
    
    osc_server_->addHandler(osc::common::PING,
        [this](const std::string& path, lo_message msg) { handlePing(msg); });
}

void NodeEditor::handleEngineStatus(lo_message msg) {
    if (lo_message_get_argc(msg) >= 1) {
        const char* status = &lo_message_get_argv(msg)[0]->s;
        std::cout << "Engine status: " << status << std::endl;
        
        if (std::string(status) == "running") {
            engine_connected_ = true;
        } else if (std::string(status) == "shutting_down") {
            engine_connected_ = false;
        }
    }
}

void NodeEditor::handleNodeCreated(lo_message msg) {
    if (lo_message_get_argc(msg) >= 3) {
        int id = lo_message_get_argv(msg)[0]->i;
        const char* name = &lo_message_get_argv(msg)[1]->s;
        const char* type = &lo_message_get_argv(msg)[2]->s;
        
        std::cout << "Node created in engine: " << id << " (" << name << ", " << type << ")" << std::endl;
        
        // Update local graph copy
        // TODO: Create proper node and add to local_graph_
        next_node_id_ = std::max(next_node_id_, id + 1);
    }
}

void NodeEditor::handleNodeDeleted(lo_message msg) {
    if (lo_message_get_argc(msg) >= 1) {
        int id = lo_message_get_argv(msg)[0]->i;
        std::cout << "Node deleted in engine: " << id << std::endl;
        
        // Update local graph copy
        local_graph_->removeNode(id);
    }
}

void NodeEditor::handleConnectionCreated(lo_message msg) {
    if (lo_message_get_argc(msg) >= 4) {
        int source_id = lo_message_get_argv(msg)[0]->i;
        const char* source_output = &lo_message_get_argv(msg)[1]->s;
        int target_id = lo_message_get_argv(msg)[2]->i;
        const char* target_input = &lo_message_get_argv(msg)[3]->s;
        
        std::cout << "Connection created in engine: " << source_id << "." << source_output 
                  << " -> " << target_id << "." << target_input << std::endl;
        
        // Update local graph copy
        // TODO: Add connection to local_graph_
    }
}

void NodeEditor::handleConnectionDeleted(lo_message msg) {
    if (lo_message_get_argc(msg) >= 1) {
        int connection_id = lo_message_get_argv(msg)[0]->i;
        std::cout << "Connection deleted in engine: " << connection_id << std::endl;
        
        // Update local graph copy
        local_graph_->removeConnection(connection_id);
    }
}

void NodeEditor::handleParameterUpdated(lo_message msg) {
    if (lo_message_get_argc(msg) >= 3) {
        int node_id = lo_message_get_argv(msg)[0]->i;
        const char* param_name = &lo_message_get_argv(msg)[1]->s;
        const char* value = &lo_message_get_argv(msg)[2]->s;
        
        std::cout << "Parameter updated in engine: node " << node_id 
                  << ", " << param_name << " = " << value << std::endl;
        
        // Update local graph copy
        // TODO: Update parameter in local_graph_
    }
}

void NodeEditor::handleQuit(lo_message msg) {
    std::cout << "Received quit message" << std::endl;
    running_ = false;
}

void NodeEditor::handlePing(lo_message msg) {
    // Respond with pong
    if (engine_connected_) {
        engine_client_->sendMessage(std::string(osc::common::PONG));
    }
    code_interpreter_client_->sendMessage(std::string(osc::common::PONG));
}

void NodeEditor::createNodeInEngine(const std::string& name, const std::string& type, float x, float y) {
    if (!engine_connected_) {
        std::cerr << "Not connected to engine" << std::endl;
        return;
    }
    
    int node_id = next_node_id_++;
    std::string message = std::to_string(node_id) + "," + name + "," + type;
    engine_client_->sendMessage(std::string(osc::engine::CREATE_NODE), message);
    
    std::cout << "Requested node creation: " << node_id << " (" << name << ", " << type 
              << ") at (" << x << ", " << y << ")" << std::endl;
}

void NodeEditor::deleteNodeInEngine(int node_id) {
    if (!engine_connected_) {
        std::cerr << "Not connected to engine" << std::endl;
        return;
    }
    
    engine_client_->sendMessage(std::string(osc::engine::DELETE_NODE), std::to_string(node_id));
    std::cout << "Requested node deletion: " << node_id << std::endl;
}

void NodeEditor::connectNodesInEngine(int source_id, const std::string& source_output,
                                     int target_id, const std::string& target_input) {
    if (!engine_connected_) {
        std::cerr << "Not connected to engine" << std::endl;
        return;
    }
    
    std::string message = std::to_string(source_id) + "," + source_output + "," + 
                         std::to_string(target_id) + "," + target_input;
    engine_client_->sendMessage(std::string(osc::engine::CONNECT_NODES), message);
    
    std::cout << "Requested connection: " << source_id << "." << source_output 
              << " -> " << target_id << "." << target_input << std::endl;
}

void NodeEditor::disconnectNodesInEngine(int connection_id) {
    if (!engine_connected_) {
        std::cerr << "Not connected to engine" << std::endl;
        return;
    }
    
    engine_client_->sendMessage(std::string(osc::engine::DISCONNECT_NODES), std::to_string(connection_id));
    std::cout << "Requested disconnection: " << connection_id << std::endl;
}

void NodeEditor::updateParameterInEngine(int node_id, const std::string& param_name, 
                                        const std::string& value) {
    if (!engine_connected_) {
        std::cerr << "Not connected to engine" << std::endl;
        return;
    }
    
    std::string message = std::to_string(node_id) + "," + param_name + "," + value;
    engine_client_->sendMessage(std::string(osc::engine::SET_PARAMETER), message);
    std::cout << "Requested parameter update: node " << node_id 
              << ", " << param_name << " = " << value << std::endl;
}

void NodeEditor::saveGraph(const std::string& filename) {
    // TODO: Implement graph saving
    std::cout << "Save graph to: " << filename << " (not implemented)" << std::endl;
}

void NodeEditor::loadGraph(const std::string& filename) {
    // TODO: Implement graph loading
    std::cout << "Load graph from: " << filename << " (not implemented)" << std::endl;
}

} // namespace gfx
