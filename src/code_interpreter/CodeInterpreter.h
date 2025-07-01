#pragma once

#include "../osc/OSCServer.h"
#include "../osc/OSCClient.h"
#include "../osc/OSCMessages.h"
#include <memory>
#include <atomic>
#include <string>
#include <map>
#include <functional>

namespace gfx {

class CodeInterpreter {
public:
    using ScriptFunction = std::function<void(const std::vector<std::string>&)>;
    
    CodeInterpreter();
    ~CodeInterpreter();
    
    // Core lifecycle
    bool initialize();
    void run();
    void shutdown();
    
    // OSC message handlers
    void handleExecuteCode(lo_message msg);
    void handleRegisterFunction(lo_message msg);
    void handleCallFunction(lo_message msg);
    void handleEngineStatus(lo_message msg);
    void handleNodeEditorStatus(lo_message msg);
    void handleQuit(lo_message msg);
    void handlePing(lo_message msg);
    
    // Script execution
    std::string executeCode(const std::string& code);
    void registerFunction(const std::string& name, ScriptFunction function);
    std::string callFunction(const std::string& name, const std::vector<std::string>& args);
    
    // Built-in functions
    void setupBuiltinFunctions();
    void createNodeFunction(const std::vector<std::string>& args);
    void deleteNodeFunction(const std::vector<std::string>& args);
    void connectNodesFunction(const std::vector<std::string>& args);
    void setParameterFunction(const std::vector<std::string>& args);
    void printFunction(const std::vector<std::string>& args);
    
    // Status
    bool isRunning() const { return running_; }
    
private:
    void setupOSCHandlers();
    void initializeAngelScript();
    void shutdownAngelScript();
    void processCommands();
    
    std::unique_ptr<OSCServer> osc_server_;
    std::unique_ptr<OSCClient> engine_client_;
    std::unique_ptr<OSCClient> node_editor_client_;
    
    std::atomic<bool> running_;
    bool engine_connected_;
    bool node_editor_connected_;
    
    // AngelScript context (placeholder for now)
    void* script_engine_;
    void* script_context_;
    
    // Function registry
    std::map<std::string, ScriptFunction> registered_functions_;
    
    // Command history
    std::vector<std::string> command_history_;
};

} // namespace gfx
