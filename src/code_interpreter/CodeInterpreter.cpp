#include "CodeInterpreter.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <thread>
#include <chrono>
#include <lo/lo.h>

namespace gfx {

CodeInterpreter::CodeInterpreter() 
    : running_(false), engine_connected_(false), node_editor_connected_(false),
      script_engine_(nullptr), script_context_(nullptr) {
    
    osc_server_ = std::make_unique<OSCServer>(osc::CODE_INTERPRETER_PORT);
    engine_client_ = std::make_unique<OSCClient>();
    node_editor_client_ = std::make_unique<OSCClient>();
}

CodeInterpreter::~CodeInterpreter() {
    shutdown();
}

bool CodeInterpreter::initialize() {
    std::cout << "Initializing Code Interpreter..." << std::endl;
    
    // Start OSC server
    if (!osc_server_->start()) {
        std::cerr << "Failed to start OSC server" << std::endl;
        return false;
    }
    
    // Setup OSC message handlers
    setupOSCHandlers();
    
    // Connect to engine and node editor
    if (engine_client_->connect("localhost", osc::ENGINE_PORT)) {
        engine_connected_ = true;
        std::cout << "Connected to Graphics Engine" << std::endl;
    } else {
        std::cout << "Graphics Engine not available (will retry)" << std::endl;
    }
    
    if (node_editor_client_->connect("localhost", osc::NODE_EDITOR_PORT)) {
        node_editor_connected_ = true;
        std::cout << "Connected to Node Editor" << std::endl;
    } else {
        std::cout << "Node Editor not available (will retry)" << std::endl;
    }
    
    // Initialize scripting engine (stub for now)
    initializeAngelScript();
    
    // Setup built-in functions
    setupBuiltinFunctions();
    
    std::cout << "Code Interpreter initialized successfully" << std::endl;
    std::cout << "OSC Server listening on port " << osc::CODE_INTERPRETER_PORT << std::endl;
    
    return true;
}

void CodeInterpreter::run() {
    if (!initialize()) {
        return;
    }
    
    running_ = true;
    
    // Send ping to other components to test connections
    if (engine_connected_) {
        engine_client_->sendMessage(osc::common::PING);
    }
    if (node_editor_connected_) {
        node_editor_client_->sendMessage(osc::common::PING);
    }
    
    std::cout << "Code Interpreter is running. Type commands or 'quit' to exit." << std::endl;
    std::cout << "Available commands: createNode, deleteNode, connectNodes, setParameter, print, quit" << std::endl;
    
    // Main command processing loop
    while (running_) {
        processCommands();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void CodeInterpreter::shutdown() {
    if (!running_) {
        return;
    }
    
    std::cout << "Shutting down Code Interpreter..." << std::endl;
    
    running_ = false;
    
    // Notify other components
    if (engine_connected_) {
        engine_client_->sendMessage(std::string(osc::code_interpreter::STATUS), std::string("shutting_down"));
    }
    if (node_editor_connected_) {
        node_editor_client_->sendMessage(std::string(osc::code_interpreter::STATUS), std::string("shutting_down"));
    }
    
    // Shutdown scripting engine
    shutdownAngelScript();
    
    // Stop OSC server
    if (osc_server_) {
        osc_server_->stop();
    }
    
    // Disconnect clients
    if (engine_client_) {
        engine_client_->disconnect();
    }
    if (node_editor_client_) {
        node_editor_client_->disconnect();
    }
    
    std::cout << "Code Interpreter shutdown complete" << std::endl;
}

void CodeInterpreter::setupOSCHandlers() {
    // Code execution
    osc_server_->addHandler(osc::code_interpreter::EXECUTE_CODE,
        [this](const std::string& path, lo_message msg) { handleExecuteCode(msg); });
    
    osc_server_->addHandler(osc::code_interpreter::REGISTER_FUNCTION,
        [this](const std::string& path, lo_message msg) { handleRegisterFunction(msg); });
    
    osc_server_->addHandler(osc::code_interpreter::CALL_FUNCTION,
        [this](const std::string& path, lo_message msg) { handleCallFunction(msg); });
    
    // Status updates from other components
    osc_server_->addHandler(osc::engine::STATUS,
        [this](const std::string& path, lo_message msg) { handleEngineStatus(msg); });
    
    osc_server_->addHandler(osc::node_editor::STATUS,
        [this](const std::string& path, lo_message msg) { handleNodeEditorStatus(msg); });
    
    // Control
    osc_server_->addHandler(osc::code_interpreter::QUIT,
        [this](const std::string& path, lo_message msg) { handleQuit(msg); });
    
    osc_server_->addHandler(osc::common::PING,
        [this](const std::string& path, lo_message msg) { handlePing(msg); });
}

void CodeInterpreter::handleExecuteCode(lo_message msg) {
    if (lo_message_get_argc(msg) >= 1) {
        const char* code = &lo_message_get_argv(msg)[0]->s;
        
        std::cout << "Executing code: " << code << std::endl;
        std::string result = executeCode(code);
        
        // Send result back via OSC
        if (engine_connected_) {
            engine_client_->sendMessage(std::string(osc::code_interpreter::EXECUTION_RESULT), result);
        }
        if (node_editor_connected_) {
            node_editor_client_->sendMessage(std::string(osc::code_interpreter::EXECUTION_RESULT), result);
        }
    }
}

void CodeInterpreter::handleRegisterFunction(lo_message msg) {
    if (lo_message_get_argc(msg) >= 1) {
        const char* function_name = &lo_message_get_argv(msg)[0]->s;
        std::cout << "Register function request: " << function_name << " (not implemented)" << std::endl;
    }
}

void CodeInterpreter::handleCallFunction(lo_message msg) {
    if (lo_message_get_argc(msg) >= 1) {
        const char* function_name = &lo_message_get_argv(msg)[0]->s;
        
        std::vector<std::string> args;
        for (int i = 1; i < lo_message_get_argc(msg); ++i) {
            args.push_back(&lo_message_get_argv(msg)[i]->s);
        }
        
        std::cout << "Calling function: " << function_name << std::endl;
        std::string result = callFunction(function_name, args);
        
        // Send result back via OSC
        if (engine_connected_) {
            engine_client_->sendMessage(std::string(osc::code_interpreter::EXECUTION_RESULT), result);
        }
        if (node_editor_connected_) {
            node_editor_client_->sendMessage(std::string(osc::code_interpreter::EXECUTION_RESULT), result);
        }
    }
}

void CodeInterpreter::handleEngineStatus(lo_message msg) {
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

void CodeInterpreter::handleNodeEditorStatus(lo_message msg) {
    if (lo_message_get_argc(msg) >= 1) {
        const char* status = &lo_message_get_argv(msg)[0]->s;
        std::cout << "Node Editor status: " << status << std::endl;
        
        if (std::string(status) == "running") {
            node_editor_connected_ = true;
        } else if (std::string(status) == "shutting_down") {
            node_editor_connected_ = false;
        }
    }
}

void CodeInterpreter::handleQuit(lo_message msg) {
    std::cout << "Received quit message" << std::endl;
    running_ = false;
}

void CodeInterpreter::handlePing(lo_message msg) {
    // Respond with pong
    if (engine_connected_) {
        engine_client_->sendMessage(std::string(osc::common::PONG));
    }
    if (node_editor_connected_) {
        node_editor_client_->sendMessage(std::string(osc::common::PONG));
    }
}

std::string CodeInterpreter::executeCode(const std::string& code) {
    // Simple command parser for now (replace with AngelScript later)
    std::istringstream iss(code);
    std::string command;
    iss >> command;
    
    std::vector<std::string> args;
    std::string arg;
    while (iss >> arg) {
        args.push_back(arg);
    }
    
    // Check if it's a registered function
    auto it = registered_functions_.find(command);
    if (it != registered_functions_.end()) {
        try {
            it->second(args);
            return "OK";
        } catch (const std::exception& e) {
            return std::string("Error: ") + e.what();
        }
    }
    
    return "Unknown command: " + command;
}

void CodeInterpreter::registerFunction(const std::string& name, ScriptFunction function) {
    registered_functions_[name] = function;
    std::cout << "Registered function: " << name << std::endl;
}

std::string CodeInterpreter::callFunction(const std::string& name, const std::vector<std::string>& args) {
    auto it = registered_functions_.find(name);
    if (it != registered_functions_.end()) {
        try {
            it->second(args);
            return "OK";
        } catch (const std::exception& e) {
            return std::string("Error: ") + e.what();
        }
    }
    
    return "Unknown function: " + name;
}

void CodeInterpreter::setupBuiltinFunctions() {
    registerFunction("createNode", [this](const std::vector<std::string>& args) { 
        createNodeFunction(args); 
    });
    
    registerFunction("deleteNode", [this](const std::vector<std::string>& args) { 
        deleteNodeFunction(args); 
    });
    
    registerFunction("connectNodes", [this](const std::vector<std::string>& args) { 
        connectNodesFunction(args); 
    });
    
    registerFunction("setParameter", [this](const std::vector<std::string>& args) { 
        setParameterFunction(args); 
    });
    
    registerFunction("print", [this](const std::vector<std::string>& args) { 
        printFunction(args); 
    });
    
    registerFunction("quit", [this](const std::vector<std::string>& args) { 
        running_ = false; 
    });
}

void CodeInterpreter::createNodeFunction(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        throw std::runtime_error("createNode requires at least 2 arguments: name type [x y]");
    }
    
    std::string name = args[0];
    std::string type = args[1];
    
    if (!engine_connected_) {
        throw std::runtime_error("Not connected to engine");
    }
    
    // Generate a node ID
    static int next_id = 1000; // Start from 1000 for interpreter-created nodes
    int node_id = next_id++;
    
    std::string message = std::to_string(node_id) + "," + name + "," + type;
    engine_client_->sendMessage(std::string(osc::engine::CREATE_NODE), message);
    
    std::cout << "Created node: " << node_id << " (" << name << ", " << type << ")" << std::endl;
}

void CodeInterpreter::deleteNodeFunction(const std::vector<std::string>& args) {
    if (args.size() < 1) {
        throw std::runtime_error("deleteNode requires 1 argument: node_id");
    }
    
    int node_id = std::stoi(args[0]);
    
    if (!engine_connected_) {
        throw std::runtime_error("Not connected to engine");
    }
    
    engine_client_->sendMessage(std::string(osc::engine::DELETE_NODE), std::to_string(node_id));
    
    std::cout << "Deleted node: " << node_id << std::endl;
}

void CodeInterpreter::connectNodesFunction(const std::vector<std::string>& args) {
    if (args.size() < 4) {
        throw std::runtime_error("connectNodes requires 4 arguments: source_id source_output target_id target_input");
    }
    
    int source_id = std::stoi(args[0]);
    std::string source_output = args[1];
    int target_id = std::stoi(args[2]);
    std::string target_input = args[3];
    
    if (!engine_connected_) {
        throw std::runtime_error("Not connected to engine");
    }
    
    std::string message = std::to_string(source_id) + "," + source_output + "," + 
                         std::to_string(target_id) + "," + target_input;
    engine_client_->sendMessage(std::string(osc::engine::CONNECT_NODES), message);
    
    std::cout << "Connected: " << source_id << "." << source_output 
              << " -> " << target_id << "." << target_input << std::endl;
}

void CodeInterpreter::setParameterFunction(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        throw std::runtime_error("setParameter requires 3 arguments: node_id param_name value");
    }
    
    int node_id = std::stoi(args[0]);
    std::string param_name = args[1];
    std::string value = args[2];
    
    if (!engine_connected_) {
        throw std::runtime_error("Not connected to engine");
    }
    
    std::string message = std::to_string(node_id) + "," + param_name + "," + value;
    engine_client_->sendMessage(std::string(osc::engine::SET_PARAMETER), message);
    
    std::cout << "Set parameter: node " << node_id << ", " << param_name << " = " << value << std::endl;
}

void CodeInterpreter::printFunction(const std::vector<std::string>& args) {
    for (const auto& arg : args) {
        std::cout << arg << " ";
    }
    std::cout << std::endl;
}

void CodeInterpreter::initializeAngelScript() {
    // TODO: Initialize AngelScript engine
    // This would include:
    // - asCreateScriptEngine()
    // - Register built-in types and functions
    // - Setup script context
    std::cout << "AngelScript initialization (stub)" << std::endl;
}

void CodeInterpreter::shutdownAngelScript() {
    // TODO: Cleanup AngelScript engine
    std::cout << "AngelScript shutdown (stub)" << std::endl;
}

void CodeInterpreter::processCommands() {
    // Simple console input processing
    // In a real implementation, this might be replaced with a REPL or GUI
    static bool shown_prompt = false;
    if (!shown_prompt) {
        std::cout << "interpreter> ";
        std::cout.flush();
        shown_prompt = true;
    }
    
    // For now, auto-execute some demo commands
    static auto last_demo = std::chrono::steady_clock::now();
    static int demo_step = 0;
    auto now = std::chrono::steady_clock::now();
    
    if (std::chrono::duration_cast<std::chrono::seconds>(now - last_demo).count() >= 3 && engine_connected_) {
        switch (demo_step % 3) {
            case 0:
                executeCode("print Hello from Code Interpreter!");
                break;
            case 1:
                executeCode("createNode test_script_node generator");
                break;
            case 2:
                executeCode("setParameter 1000 amplitude 0.5");
                break;
        }
        demo_step++;
        last_demo = now;
    }
}

} // namespace gfx
