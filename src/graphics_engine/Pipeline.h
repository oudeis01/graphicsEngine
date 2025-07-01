#pragma once

#include "../core/NodeGraph.h"
#include <string>
#include <memory>

namespace gfx {

class ShaderManager;

/**
 * @brief Graphics rendering pipeline management
 * 
 * Manages the graphics pipeline state, node graph processing,
 * and coordinates between OSC messages and shader generation.
 */
class Pipeline {
public:
    Pipeline();
    ~Pipeline();
    
    /**
     * @brief Initialize pipeline with shader manager
     * @param shaderManager Shared shader manager instance
     * @return true if initialization successful, false otherwise
     */
    bool initialize(std::shared_ptr<ShaderManager> shaderManager);
    
    /**
     * @brief Shutdown and cleanup pipeline
     */
    void shutdown();
    
    /**
     * @brief Update pipeline from node graph
     * @param nodeGraph Node graph to process
     * @return true if update successful, false otherwise
     */
    bool updateFromNodeGraph(const NodeGraph& nodeGraph);
    
    /**
     * @brief Update pipeline from OSC message string
     * @param pipelineData Pipeline data as string
     * @return true if update successful, false otherwise
     */
    bool updateFromString(const std::string& pipelineData);
    
    /**
     * @brief Render current pipeline
     * @param deltaTime Time since last frame
     */
    void render(float deltaTime);
    
    /**
     * @brief Set pipeline parameter
     * @param nodeId Target node ID
     * @param paramName Parameter name
     * @param value Parameter value
     * @return true if parameter set successfully, false otherwise
     */
    bool setParameter(int nodeId, const std::string& paramName, const std::string& value);
    
    /**
     * @brief Get current pipeline as string representation
     * @return Pipeline data as string
     */
    std::string getPipelineString() const;
    
    /**
     * @brief Check if pipeline is valid and ready to render
     * @return true if ready, false otherwise
     */
    bool isReady() const;
    
    /**
     * @brief Get current node graph
     * @return Reference to current node graph
     */
    const NodeGraph& getNodeGraph() const { return node_graph_; }

private:
    /**
     * @brief Generate shader from current node graph
     * @return true if generation successful, false otherwise
     */
    bool generateShader();
    
    /**
     * @brief Setup default rendering quad
     */
    void setupQuad();
    
    /**
     * @brief Cleanup rendering resources
     */
    void cleanupQuad();
    
    /**
     * @brief Update uniform values for current frame
     * @param deltaTime Time since last frame
     */
    void updateUniforms(float deltaTime);
    
    std::shared_ptr<ShaderManager> shader_manager_;  ///< Shader manager instance
    NodeGraph node_graph_;                           ///< Current node graph
    unsigned int shader_program_;                    ///< Current shader program ID
    unsigned int vao_, vbo_, ebo_;                  ///< Rendering quad geometry
    bool initialized_;                               ///< Initialization state
    float total_time_;                              ///< Total elapsed time
};

} // namespace gfx
