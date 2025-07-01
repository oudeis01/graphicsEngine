#include "Pipeline.h"
#include "PipelineGraph.h"
#include "RenderContext.h"
#include "ShaderManager.h"
#include <iostream>
#include <sstream>
#include <algorithm>

/**
 * @brief Internal implementation structure for Pipeline
 * Uses PIMPL idiom to hide implementation details
 */
struct Pipeline::Impl {
    std::unique_ptr<PipelineGraph> graph;           ///< The pipeline graph
    std::unique_ptr<RenderContext> renderContext;   ///< Render context
    std::unique_ptr<ShaderManager> shaderManager;   ///< Shader manager
    
    // Execution state
    GLuint currentProgram = 0;                      ///< Currently bound shader program
    bool isExecuting = false;                       ///< Execution state flag
    
    Impl() {
        graph = std::make_unique<PipelineGraph>();
    }
};

/**
 * @brief Constructor
 */
Pipeline::Pipeline() : impl(std::make_unique<Impl>()) {
    std::cout << "Pipeline created" << std::endl;
}

/**
 * @brief Destructor
 */
Pipeline::~Pipeline() {
    std::cout << "Pipeline destroyed" << std::endl;
}

bool Pipeline::initialize(RenderContext* context, ShaderManager* shaderMgr) {
    if (!context || !shaderMgr) {
        std::cerr << "Pipeline::initialize - Invalid context or shader manager" << std::endl;
        return false;
    }
    
    impl->renderContext.reset(context);
    impl->shaderManager.reset(shaderMgr);
    
    std::cout << "Pipeline initialized successfully" << std::endl;
    return true;
}

void Pipeline::cleanup() {
    impl->graph->clear();
    impl->currentProgram = 0;
    impl->isExecuting = false;
    
    std::cout << "Pipeline cleaned up" << std::endl;
}

// ====================================================
// Node Management
// ====================================================

int Pipeline::addNode(const std::string& type, const std::unordered_map<std::string, std::string>& parameters) {
    return impl->graph->addNode(type, parameters);
}

bool Pipeline::removeNode(int nodeId) {
    return impl->graph->removeNode(nodeId);
}

bool Pipeline::connectNodes(int fromNodeId, const std::string& fromPort, int toNodeId, const std::string& toPort) {
    return impl->graph->connect(fromNodeId, fromPort, toNodeId, toPort);
}

bool Pipeline::disconnectNodes(int fromNodeId, const std::string& fromPort, int toNodeId, const std::string& toPort) {
    return impl->graph->disconnect(fromNodeId, fromPort, toNodeId, toPort);
}

bool Pipeline::setOutput(int nodeId, const std::string& port) {
    return impl->graph->setOutput(nodeId, port);
}

// ====================================================
// Execution
// ====================================================

bool Pipeline::compile() {
    if (!impl->shaderManager) {
        std::cerr << "Pipeline::compile - No shader manager available" << std::endl;
        return false;
    }
    
    // Validate the graph first
    auto validation = impl->graph->validateGraph();
    if (!validation.isValid) {
        std::cerr << "Pipeline::compile - Graph validation failed:" << std::endl;
        for (const auto& error : validation.errors) {
            std::cerr << "  Error: " << error << std::endl;
        }
        return false;
    }
    
    // Print warnings if any
    for (const auto& warning : validation.warnings) {
        std::cout << "  Warning: " << warning << std::endl;
    }
    
    // Generate shader from graph
    auto result = impl->shaderManager->generateShaderFromGraph(*impl->graph);
    if (!result.success) {
        std::cerr << "Pipeline::compile - Shader generation failed: " << result.errorLog << std::endl;
        return false;
    }
    
    impl->currentProgram = result.program;
    std::cout << "Pipeline compiled successfully. Program ID: " << impl->currentProgram << std::endl;
    
    return true;
}

bool Pipeline::execute() {
    if (impl->currentProgram == 0) {
        std::cerr << "Pipeline::execute - No compiled program available. Call compile() first." << std::endl;
        return false;
    }
    
    if (!impl->shaderManager || !impl->renderContext) {
        std::cerr << "Pipeline::execute - Missing dependencies" << std::endl;
        return false;
    }
    
    impl->isExecuting = true;
    
    // Use the compiled shader
    impl->shaderManager->useShader(impl->currentProgram);
    
    // Set common uniforms
    impl->shaderManager->setUniform("iTime", static_cast<float>(getCurrentTime()));
    impl->shaderManager->setUniform("iResolution", 
        static_cast<float>(impl->renderContext->getViewportWidth()), 
        static_cast<float>(impl->renderContext->getViewportHeight()));
    
    // Render fullscreen quad
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Note: Actual fullscreen quad rendering would be handled by RenderContext
    // For now, we just indicate successful execution
    
    impl->isExecuting = false;
    return true;
}

bool Pipeline::isValid() const {
    auto validation = impl->graph->validateGraph();
    return validation.isValid;
}

// ====================================================
// Serialization (Simple DSL-like format)
// ====================================================

std::string Pipeline::serialize() const {
    return impl->graph->toDSL();
}

bool Pipeline::deserialize(const std::string& data) {
    return impl->graph->fromDSL(data);
}

// ====================================================
// Graph Access
// ====================================================

const PipelineGraph& Pipeline::getGraph() const {
    return *impl->graph;
}

PipelineGraph& Pipeline::getGraph() {
    return *impl->graph;
}

// ====================================================
// Debug and Monitoring
// ====================================================

void Pipeline::printGraphInfo() const {
    std::cout << "=== Pipeline Graph Info ===" << std::endl;
    std::cout << "Nodes: " << impl->graph->getNodes().size() << std::endl;
    std::cout << "Connections: " << impl->graph->getConnections().size() << std::endl;
    std::cout << "Output Node ID: " << impl->graph->getOutputNodeId() << std::endl;
    std::cout << "Output Port: " << impl->graph->getOutputPort() << std::endl;
    std::cout << "Valid: " << (isValid() ? "Yes" : "No") << std::endl;
    std::cout << "Compiled: " << (impl->currentProgram != 0 ? "Yes" : "No") << std::endl;
    std::cout << "===========================" << std::endl;
}

GLuint Pipeline::getCurrentProgram() const {
    return impl->currentProgram;
}

bool Pipeline::isExecuting() const {
    return impl->isExecuting;
}

// ====================================================
// Helper Functions
// ====================================================

double Pipeline::getCurrentTime() const {
    // Simple time implementation - in a real application this would be more sophisticated
    static auto startTime = std::chrono::steady_clock::now();
    auto currentTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime);
    return duration.count() / 1000.0;
}
