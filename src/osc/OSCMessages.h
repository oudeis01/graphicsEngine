#pragma once

#include <string>

namespace gfx {
namespace osc {

// Standard OSC ports for each component
constexpr int ENGINE_PORT = 57120;
constexpr int NODE_EDITOR_PORT = 57121;
constexpr int CODE_INTERPRETER_PORT = 57122;

// Message paths for Engine
namespace engine {
    constexpr const char* STATUS = "/engine/status";
    constexpr const char* QUIT = "/engine/quit";
    constexpr const char* CREATE_NODE = "/engine/node/create";
    constexpr const char* DELETE_NODE = "/engine/node/delete";
    constexpr const char* UPDATE_NODE = "/engine/node/update";
    constexpr const char* SET_PARAMETER = "/engine/node/param/set";
    constexpr const char* GET_PARAMETER = "/engine/node/param/get";
    constexpr const char* CONNECT_NODES = "/engine/connection/create";
    constexpr const char* DISCONNECT_NODES = "/engine/connection/delete";
    constexpr const char* RENDER_FRAME = "/engine/render";
}

// Message paths for Node Editor
namespace node_editor {
    constexpr const char* STATUS = "/editor/status";
    constexpr const char* QUIT = "/editor/quit";
    constexpr const char* NODE_SELECTED = "/editor/node/selected";
    constexpr const char* NODE_MOVED = "/editor/node/moved";
    constexpr const char* CONNECTION_CREATED = "/editor/connection/created";
    constexpr const char* CONNECTION_DELETED = "/editor/connection/deleted";
    constexpr const char* PARAMETER_CHANGED = "/editor/parameter/changed";
    constexpr const char* SAVE_GRAPH = "/editor/graph/save";
    constexpr const char* LOAD_GRAPH = "/editor/graph/load";
}

// Message paths for Code Interpreter
namespace code_interpreter {
    constexpr const char* STATUS = "/interpreter/status";
    constexpr const char* QUIT = "/interpreter/quit";
    constexpr const char* EXECUTE_CODE = "/interpreter/execute";
    constexpr const char* EXECUTION_RESULT = "/interpreter/result";
    constexpr const char* EXECUTION_ERROR = "/interpreter/error";
    constexpr const char* REGISTER_FUNCTION = "/interpreter/function/register";
    constexpr const char* CALL_FUNCTION = "/interpreter/function/call";
}

// Common message paths (used by all components)
namespace common {
    constexpr const char* PING = "/ping";
    constexpr const char* PONG = "/pong";
    constexpr const char* ERROR = "/error";
    constexpr const char* LOG = "/log";
}

// Node types
enum class NodeType {
    SOURCE,
    EFFECT,
    GENERATOR,
    COMPOSITE,
    OUTPUT,
    CUSTOM
};

std::string nodeTypeToString(NodeType type);
NodeType stringToNodeType(const std::string& str);

// Parameter types
enum class ParameterType {
    INT,
    FLOAT,
    STRING,
    BOOL,
    VEC2,
    VEC3,
    VEC4,
    COLOR
};

std::string parameterTypeToString(ParameterType type);
ParameterType stringToParameterType(const std::string& str);

} // namespace osc
} // namespace gfx
