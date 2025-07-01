#include "GeneratorModules.h"
#include "PipelineGraph.h"
#include <sstream>

// ============================================================================
// NoiseGenerator Implementation
// ============================================================================

NoiseGenerator::NoiseGenerator() : PipelineModule("noise", ModuleType::GENERATOR) {
    // Configure input ports
    inputPorts_ = {
        {"uv", DataType::VEC2, true, ""},
        {"scale", DataType::FLOAT, false, "3.0"},
        {"time", DataType::FLOAT, false, "0.0"}
    };
    
    // Configure output ports
    outputPorts_ = {
        {"output", DataType::FLOAT, true, ""}
    };
}

std::string NoiseGenerator::generateGLSL(
    const std::unordered_map<std::string, std::string>& inputs,
    const std::unordered_map<std::string, std::string>& outputs
) const {
    std::stringstream glsl;
    
    std::string uvVar = inputs.count("uv") ? inputs.at("uv") : "uv";
    std::string scaleVar = inputs.count("scale") ? inputs.at("scale") : "3.0";
    std::string timeVar = inputs.count("time") ? inputs.at("time") : "0.0";
    std::string outputVar = outputs.count("output") ? outputs.at("output") : "noiseOutput";
    
    glsl << "    // Simplex Noise Generation\n";
    glsl << "    float " << outputVar << " = snoise(" << uvVar << " * " << scaleVar 
         << " + " << timeVar << " * 0.1);\n";
    
    return glsl.str();
}

std::vector<std::string> NoiseGenerator::getRequiredIncludes() const {
    return {"generative/snoise.glsl"};
}

std::unordered_map<std::string, std::string> NoiseGenerator::getParameters() const {
    return {
        {"scale", "3.0"},
        {"timeMultiplier", "0.1"}
    };
}

// ============================================================================
// VoronoiGenerator Implementation
// ============================================================================

VoronoiGenerator::VoronoiGenerator() : PipelineModule("voronoi", ModuleType::GENERATOR) {
    inputPorts_ = {
        {"uv", DataType::VEC2, true, ""},
        {"scale", DataType::FLOAT, false, "5.0"},
        {"time", DataType::FLOAT, false, "0.0"}
    };
    
    outputPorts_ = {
        {"distance", DataType::FLOAT, true, ""},
        {"cellId", DataType::FLOAT, true, ""}
    };
}

std::string VoronoiGenerator::generateGLSL(
    const std::unordered_map<std::string, std::string>& inputs,
    const std::unordered_map<std::string, std::string>& outputs
) const {
    std::stringstream glsl;
    
    std::string uvVar = inputs.count("uv") ? inputs.at("uv") : "uv";
    std::string scaleVar = inputs.count("scale") ? inputs.at("scale") : "5.0";
    std::string timeVar = inputs.count("time") ? inputs.at("time") : "0.0";
    std::string distVar = outputs.count("distance") ? outputs.at("distance") : "voronoiDist";
    std::string cellVar = outputs.count("cellId") ? outputs.at("cellId") : "voronoiCell";
    
    glsl << "    // Voronoi Pattern Generation\n";
    glsl << "    vec3 voronoiResult = voronoi(" << uvVar << " * " << scaleVar 
         << " + " << timeVar << " * 0.05);\n";
    glsl << "    float " << distVar << " = voronoiResult.x;\n";
    glsl << "    float " << cellVar << " = voronoiResult.y;\n";
    
    return glsl.str();
}

std::vector<std::string> VoronoiGenerator::getRequiredIncludes() const {
    return {"generative/voronoi.glsl"};
}

std::unordered_map<std::string, std::string> VoronoiGenerator::getParameters() const {
    return {
        {"scale", "5.0"},
        {"timeMultiplier", "0.05"}
    };
}

// ============================================================================
// GradientGenerator Implementation
// ============================================================================

GradientGenerator::GradientGenerator() : PipelineModule("gradient", ModuleType::GENERATOR) {
    inputPorts_ = {
        {"uv", DataType::VEC2, true, ""},
        {"direction", DataType::VEC2, false, "vec2(1.0, 0.0)"},
        {"center", DataType::VEC2, false, "vec2(0.5)"}
    };
    
    outputPorts_ = {
        {"output", DataType::FLOAT, true, ""}
    };
}

std::string GradientGenerator::generateGLSL(
    const std::unordered_map<std::string, std::string>& inputs,
    const std::unordered_map<std::string, std::string>& outputs
) const {
    std::stringstream glsl;
    
    std::string uvVar = inputs.count("uv") ? inputs.at("uv") : "uv";
    std::string dirVar = inputs.count("direction") ? inputs.at("direction") : "vec2(1.0, 0.0)";
    std::string centerVar = inputs.count("center") ? inputs.at("center") : "vec2(0.5)";
    std::string outputVar = outputs.count("output") ? outputs.at("output") : "gradientOutput";
    
    glsl << "    // Linear Gradient Generation\n";
    glsl << "    vec2 gradientUV = " << uvVar << " - " << centerVar << ";\n";
    glsl << "    float " << outputVar << " = dot(gradientUV, normalize(" << dirVar << "));\n";
    glsl << "    " << outputVar << " = " << outputVar << " * 0.5 + 0.5;\n"; // Normalize to 0-1
    
    return glsl.str();
}

std::vector<std::string> GradientGenerator::getRequiredIncludes() const {
    return {}; // No special includes needed for basic gradients
}

std::unordered_map<std::string, std::string> GradientGenerator::getParameters() const {
    return {
        {"direction", "vec2(1.0, 0.0)"},
        {"center", "vec2(0.5)"}
    };
}

// ============================================================================
// RandomGenerator Implementation
// ============================================================================

RandomGenerator::RandomGenerator() : PipelineModule("random", ModuleType::GENERATOR) {
    inputPorts_ = {
        {"uv", DataType::VEC2, true, ""},
        {"scale", DataType::FLOAT, false, "10.0"},
        {"seed", DataType::FLOAT, false, "0.0"}
    };
    
    outputPorts_ = {
        {"output", DataType::FLOAT, true, ""}
    };
}

std::string RandomGenerator::generateGLSL(
    const std::unordered_map<std::string, std::string>& inputs,
    const std::unordered_map<std::string, std::string>& outputs
) const {
    std::stringstream glsl;
    
    std::string uvVar = inputs.count("uv") ? inputs.at("uv") : "uv";
    std::string scaleVar = inputs.count("scale") ? inputs.at("scale") : "10.0";
    std::string seedVar = inputs.count("seed") ? inputs.at("seed") : "0.0";
    std::string outputVar = outputs.count("output") ? outputs.at("output") : "randomOutput";
    
    glsl << "    // Random Pattern Generation\n";
    glsl << "    vec2 gridPos = floor(" << uvVar << " * " << scaleVar << ");\n";
    glsl << "    float " << outputVar << " = random(gridPos + " << seedVar << ");\n";
    
    return glsl.str();
}

std::vector<std::string> RandomGenerator::getRequiredIncludes() const {
    return {"generative/random.glsl"};
}

std::unordered_map<std::string, std::string> RandomGenerator::getParameters() const {
    return {
        {"scale", "10.0"},
        {"seed", "0.0"}
    };
}

// ============================================================================
// FBMGenerator Implementation
// ============================================================================

FBMGenerator::FBMGenerator() : PipelineModule("fbm", ModuleType::GENERATOR) {
    inputPorts_ = {
        {"uv", DataType::VEC2, true, ""},
        {"octaves", DataType::FLOAT, false, "4.0"},
        {"scale", DataType::FLOAT, false, "3.0"},
        {"time", DataType::FLOAT, false, "0.0"}
    };
    
    outputPorts_ = {
        {"output", DataType::FLOAT, true, ""}
    };
}

std::string FBMGenerator::generateGLSL(
    const std::unordered_map<std::string, std::string>& inputs,
    const std::unordered_map<std::string, std::string>& outputs
) const {
    std::stringstream glsl;
    
    std::string uvVar = inputs.count("uv") ? inputs.at("uv") : "uv";
    std::string octavesVar = inputs.count("octaves") ? inputs.at("octaves") : "4.0";
    std::string scaleVar = inputs.count("scale") ? inputs.at("scale") : "3.0";
    std::string timeVar = inputs.count("time") ? inputs.at("time") : "0.0";
    std::string outputVar = outputs.count("output") ? outputs.at("output") : "fbmOutput";
    
    glsl << "    // FBM (Fractal Brownian Motion) Generation\n";
    glsl << "    float " << outputVar << " = fbm(" << uvVar << " * " << scaleVar 
         << " + " << timeVar << " * 0.1);\n";
    
    return glsl.str();
}

std::vector<std::string> FBMGenerator::getRequiredIncludes() const {
    return {"generative/fbm.glsl"};
}

std::unordered_map<std::string, std::string> FBMGenerator::getParameters() const {
    return {
        {"octaves", "4.0"},
        {"scale", "3.0"},
        {"timeMultiplier", "0.1"}
    };
}

// ============================================================================
// Module Registration
// ============================================================================

REGISTER_MODULE(NoiseGenerator, "noise");
REGISTER_MODULE(VoronoiGenerator, "voronoi");
REGISTER_MODULE(GradientGenerator, "gradient");
REGISTER_MODULE(RandomGenerator, "random");
REGISTER_MODULE(FBMGenerator, "fbm");

// ============================================================================
// GeneratorModules Management Class Implementation
// ============================================================================

GeneratorModules::GeneratorModules() {
    // Register all available generators
    generators_["noise"] = []() { return std::make_unique<NoiseGenerator>(); };
    generators_["voronoi"] = []() { return std::make_unique<VoronoiGenerator>(); };
    generators_["gradient"] = []() { return std::make_unique<GradientGenerator>(); };
    generators_["random"] = []() { return std::make_unique<RandomGenerator>(); };
    generators_["fbm"] = []() { return std::make_unique<FBMGenerator>(); };
}

bool GeneratorModules::hasGenerator(const std::string& type) const {
    return generators_.find(type) != generators_.end();
}

std::string GeneratorModules::generateFunction(const PipelineGraph::Node& node) const {
    auto it = generators_.find(node.getType());
    if (it == generators_.end()) {
        return "// Unknown generator type: " + node.getType();
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
    
    // Generate the module's GLSL code
    std::string moduleCode = module->generateGLSL(inputs, outputs);
    function << moduleCode;
    
    // Convert single float to vec4 if needed
    if (node.getType() == "noise" || node.getType() == "fbm" || node.getType() == "random") {
        function << "    vec4 " << node.getName() << "_result = vec4(" << node.getName() << "_output);\n";
        function << "    return " << node.getName() << "_result;\n";
    } else {
        function << "    return " << node.getName() << "_output;\n";
    }
    function << "}\n";
    
    return function.str();
}

std::vector<std::string> GeneratorModules::getAvailableTypes() const {
    std::vector<std::string> types;
    for (const auto& [type, factory] : generators_) {
        types.push_back(type);
    }
    return types;
}
