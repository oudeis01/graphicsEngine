#include "OperatorModules.h"
#include "PipelineGraph.h"
#include <sstream>

// ============================================================================
// BlendOperator Implementation
// ============================================================================

BlendOperator::BlendOperator() : PipelineModule("blend", ModuleType::OPERATOR) {
    inputPorts_ = {
        {"base", DataType::VEC3, true, ""},
        {"overlay", DataType::VEC3, true, ""},
        {"opacity", DataType::FLOAT, false, "1.0"}
    };
    
    outputPorts_ = {
        {"output", DataType::VEC3, true, ""}
    };
}

std::string BlendOperator::generateGLSL(
    const std::unordered_map<std::string, std::string>& inputs,
    const std::unordered_map<std::string, std::string>& outputs
) const {
    std::stringstream glsl;
    
    std::string baseVar = inputs.count("base") ? inputs.at("base") : "vec3(0.0)";
    std::string overlayVar = inputs.count("overlay") ? inputs.at("overlay") : "vec3(0.0)";
    std::string opacityVar = inputs.count("opacity") ? inputs.at("opacity") : "1.0";
    std::string outputVar = outputs.count("output") ? outputs.at("output") : "blendOutput";
    
    glsl << "    // Blend Operation\n";
    glsl << "    vec3 " << outputVar << " = blendNormal(" << baseVar << ", " 
         << overlayVar << ", " << opacityVar << ");\n";
    
    return glsl.str();
}

std::vector<std::string> BlendOperator::getRequiredIncludes() const {
    return {"color/blend.glsl"};
}

std::unordered_map<std::string, std::string> BlendOperator::getParameters() const {
    return {
        {"opacity", "1.0"},
        {"blendMode", "normal"}
    };
}

// ============================================================================
// TransformOperator Implementation
// ============================================================================

TransformOperator::TransformOperator() : PipelineModule("transform", ModuleType::OPERATOR) {
    inputPorts_ = {
        {"uv", DataType::VEC2, true, ""},
        {"rotation", DataType::FLOAT, false, "0.0"},
        {"scale", DataType::VEC2, false, "vec2(1.0)"},
        {"offset", DataType::VEC2, false, "vec2(0.0)"}
    };
    
    outputPorts_ = {
        {"output", DataType::VEC2, true, ""}
    };
}

std::string TransformOperator::generateGLSL(
    const std::unordered_map<std::string, std::string>& inputs,
    const std::unordered_map<std::string, std::string>& outputs
) const {
    std::stringstream glsl;
    
    std::string uvVar = inputs.count("uv") ? inputs.at("uv") : "uv";
    std::string rotVar = inputs.count("rotation") ? inputs.at("rotation") : "0.0";
    std::string scaleVar = inputs.count("scale") ? inputs.at("scale") : "vec2(1.0)";
    std::string offsetVar = inputs.count("offset") ? inputs.at("offset") : "vec2(0.0)";
    std::string outputVar = outputs.count("output") ? outputs.at("output") : "transformOutput";
    
    glsl << "    // UV Transform Operation\n";
    glsl << "    vec2 centeredUV = " << uvVar << " - 0.5;\n";
    glsl << "    centeredUV = rotate2d(" << rotVar << ") * centeredUV;\n";
    glsl << "    centeredUV *= " << scaleVar << ";\n";
    glsl << "    vec2 " << outputVar << " = centeredUV + 0.5 + " << offsetVar << ";\n";
    
    return glsl.str();
}

std::vector<std::string> TransformOperator::getRequiredIncludes() const {
    return {"math/rotate2d.glsl"};
}

std::unordered_map<std::string, std::string> TransformOperator::getParameters() const {
    return {
        {"rotation", "0.0"},
        {"scale", "vec2(1.0)"},
        {"offset", "vec2(0.0)"}
    };
}

// ============================================================================
// MathOperator Implementation
// ============================================================================

MathOperator::MathOperator() : PipelineModule("math", ModuleType::OPERATOR) {
    inputPorts_ = {
        {"a", DataType::FLOAT, true, ""},
        {"b", DataType::FLOAT, true, ""},
        {"operation", DataType::FLOAT, false, "0.0"} // 0=add, 1=mul, 2=pow, etc.
    };
    
    outputPorts_ = {
        {"output", DataType::FLOAT, true, ""}
    };
}

std::string MathOperator::generateGLSL(
    const std::unordered_map<std::string, std::string>& inputs,
    const std::unordered_map<std::string, std::string>& outputs
) const {
    std::stringstream glsl;
    
    std::string aVar = inputs.count("a") ? inputs.at("a") : "0.0";
    std::string bVar = inputs.count("b") ? inputs.at("b") : "0.0";
    std::string outputVar = outputs.count("output") ? outputs.at("output") : "mathOutput";
    
    glsl << "    // Math Operation (default: add)\n";
    glsl << "    float " << outputVar << " = " << aVar << " + " << bVar << ";\n";
    
    return glsl.str();
}

std::vector<std::string> MathOperator::getRequiredIncludes() const {
    return {}; // Basic math operations don't need special includes
}

std::unordered_map<std::string, std::string> MathOperator::getParameters() const {
    return {
        {"operation", "add"}
    };
}

// ============================================================================
// ColorOperator Implementation
// ============================================================================

ColorOperator::ColorOperator() : PipelineModule("color", ModuleType::OPERATOR) {
    inputPorts_ = {
        {"value", DataType::FLOAT, true, ""},
        {"colorA", DataType::VEC3, false, "vec3(0.0)"},
        {"colorB", DataType::VEC3, false, "vec3(1.0)"}
    };
    
    outputPorts_ = {
        {"output", DataType::VEC3, true, ""}
    };
}

std::string ColorOperator::generateGLSL(
    const std::unordered_map<std::string, std::string>& inputs,
    const std::unordered_map<std::string, std::string>& outputs
) const {
    std::stringstream glsl;
    
    std::string valueVar = inputs.count("value") ? inputs.at("value") : "0.0";
    std::string colorAVar = inputs.count("colorA") ? inputs.at("colorA") : "vec3(0.0)";
    std::string colorBVar = inputs.count("colorB") ? inputs.at("colorB") : "vec3(1.0)";
    std::string outputVar = outputs.count("output") ? outputs.at("output") : "colorOutput";
    
    glsl << "    // Color Mapping Operation\n";
    glsl << "    vec3 " << outputVar << " = mix(" << colorAVar << ", " << colorBVar 
         << ", " << valueVar << ");\n";
    
    return glsl.str();
}

std::vector<std::string> ColorOperator::getRequiredIncludes() const {
    return {}; // Basic color operations use built-in GLSL functions
}

std::unordered_map<std::string, std::string> ColorOperator::getParameters() const {
    return {
        {"colorA", "vec3(0.0, 0.0, 0.0)"},
        {"colorB", "vec3(1.0, 1.0, 1.0)"}
    };
}

// ============================================================================
// FilterOperator Implementation
// ============================================================================

FilterOperator::FilterOperator() : PipelineModule("filter", ModuleType::OPERATOR) {
    inputPorts_ = {
        {"input", DataType::VEC3, true, ""},
        {"strength", DataType::FLOAT, false, "1.0"}
    };
    
    outputPorts_ = {
        {"output", DataType::VEC3, true, ""}
    };
}

std::string FilterOperator::generateGLSL(
    const std::unordered_map<std::string, std::string>& inputs,
    const std::unordered_map<std::string, std::string>& outputs
) const {
    std::stringstream glsl;
    
    std::string inputVar = inputs.count("input") ? inputs.at("input") : "vec3(0.0)";
    std::string strengthVar = inputs.count("strength") ? inputs.at("strength") : "1.0";
    std::string outputVar = outputs.count("output") ? outputs.at("output") : "filterOutput";
    
    glsl << "    // Filter Operation (default: identity)\n";
    glsl << "    vec3 " << outputVar << " = " << inputVar << ";\n";
    
    return glsl.str();
}

std::vector<std::string> FilterOperator::getRequiredIncludes() const {
    return {}; // Will be extended based on specific filter types
}

std::unordered_map<std::string, std::string> FilterOperator::getParameters() const {
    return {
        {"strength", "1.0"},
        {"filterType", "identity"}
    };
}

// ============================================================================
// Module Registration
// ============================================================================

REGISTER_MODULE(BlendOperator, "blend");
REGISTER_MODULE(TransformOperator, "transform");
REGISTER_MODULE(MathOperator, "math");
REGISTER_MODULE(ColorOperator, "color");
REGISTER_MODULE(FilterOperator, "filter");

// ============================================================================
// OperatorModules Management Class Implementation
// ============================================================================

OperatorModules::OperatorModules() {
    // Register all available operators
    operators_["blend"] = []() { return std::make_unique<BlendOperator>(); };
    operators_["transform"] = []() { return std::make_unique<TransformOperator>(); };
    operators_["math"] = []() { return std::make_unique<MathOperator>(); };
    operators_["color"] = []() { return std::make_unique<ColorOperator>(); };
    operators_["filter"] = []() { return std::make_unique<FilterOperator>(); };
}

bool OperatorModules::hasOperator(const std::string& type) const {
    return operators_.find(type) != operators_.end();
}

std::string OperatorModules::generateFunction(const PipelineGraph::Node& node) const {
    auto it = operators_.find(node.getType());
    if (it == operators_.end()) {
        return "// Unknown operator type: " + node.getType();
    }
    
    // Create temporary module instance
    auto module = it->second();
    
    // Prepare input/output mappings
    std::unordered_map<std::string, std::string> inputs;
    std::unordered_map<std::string, std::string> outputs;
    
    // Standard inputs
    inputs["uv"] = "uv";
    inputs["time"] = "iTime";
    
    // Add node parameters as inputs
    for (const auto& [key, value] : node.getParameters()) {
        inputs[key] = value;
    }
    
    // Standard output
    outputs["output"] = node.getName() + "_output";
    
    // Generate function declaration and body
    std::stringstream function;
    function << "// Function for node " << node.getId() << " (" << node.getType() << ")\n";
    function << "vec4 " << node.getName() << "_func(vec2 uv, float iTime) {\n";
    function << "    vec4 " << node.getName() << "_output = vec4(0.0);\n";
    function << module->generateGLSL(inputs, outputs);
    function << "    return " << node.getName() << "_output;\n";
    function << "}\n";
    
    return function.str();
}

std::vector<std::string> OperatorModules::getAvailableTypes() const {
    std::vector<std::string> types;
    for (const auto& [type, factory] : operators_) {
        types.push_back(type);
    }
    return types;
}
