#include "OSCMessages.h"
#include <unordered_map>

namespace gfx {
namespace osc {

std::string nodeTypeToString(NodeType type) {
    switch (type) {
        case NodeType::SOURCE: return "source";
        case NodeType::EFFECT: return "effect";
        case NodeType::GENERATOR: return "generator";
        case NodeType::COMPOSITE: return "composite";
        case NodeType::OUTPUT: return "output";
        case NodeType::CUSTOM: return "custom";
        default: return "unknown";
    }
}

NodeType stringToNodeType(const std::string& str) {
    static std::unordered_map<std::string, NodeType> typeMap = {
        {"source", NodeType::SOURCE},
        {"effect", NodeType::EFFECT},
        {"generator", NodeType::GENERATOR},
        {"composite", NodeType::COMPOSITE},
        {"output", NodeType::OUTPUT},
        {"custom", NodeType::CUSTOM}
    };
    
    auto it = typeMap.find(str);
    return (it != typeMap.end()) ? it->second : NodeType::CUSTOM;
}

std::string parameterTypeToString(ParameterType type) {
    switch (type) {
        case ParameterType::INT: return "int";
        case ParameterType::FLOAT: return "float";
        case ParameterType::STRING: return "string";
        case ParameterType::BOOL: return "bool";
        case ParameterType::VEC2: return "vec2";
        case ParameterType::VEC3: return "vec3";
        case ParameterType::VEC4: return "vec4";
        case ParameterType::COLOR: return "color";
        default: return "unknown";
    }
}

ParameterType stringToParameterType(const std::string& str) {
    static std::unordered_map<std::string, ParameterType> typeMap = {
        {"int", ParameterType::INT},
        {"float", ParameterType::FLOAT},
        {"string", ParameterType::STRING},
        {"bool", ParameterType::BOOL},
        {"vec2", ParameterType::VEC2},
        {"vec3", ParameterType::VEC3},
        {"vec4", ParameterType::VEC4},
        {"color", ParameterType::COLOR}
    };
    
    auto it = typeMap.find(str);
    return (it != typeMap.end()) ? it->second : ParameterType::FLOAT;
}

} // namespace osc
} // namespace gfx
