#pragma once

#include <memory>
#include <functional>
#include <string>

class RenderContext;
class ShaderManager;
class Pipeline;

/**
 * @brief Main graphics engine class for OpenGL-based rendering
 * 
 * Responsibilities:
 * - OpenGL context creation and management
 * - Rendering loop coordination
 * - Shader manager and pipeline integration
 * - Foundation for graphics API abstraction
 */
class GraphicsEngine {
public:
    GraphicsEngine();
    ~GraphicsEngine();

    // Core initialization and cleanup
    /**
     * @brief Initialize graphics engine with window and OpenGL context
     * @param width Initial window width in pixels
     * @param height Initial window height in pixels
     * @param title Window title string
     * @return true if initialization successful, false otherwise
     */
    bool initialize(int width, int height, const char* title);
    
    /**
     * @brief Clean up all graphics resources and terminate GLFW
     */
    void cleanup();
    
    /**
     * @brief Request main loop termination
     */
    void stop();

    // Main rendering loop
    /**
     * @brief Main rendering loop - continues until window should close
     */
    void mainLoop();
    
    /**
     * @brief Render a single frame (for node editor usage)
     */
    void renderFrame();

    // Pipeline management
    /**
     * @brief Set the active graphics pipeline
     * @param pipeline Shared pointer to the pipeline to activate
     */
    void setPipeline(std::shared_ptr<Pipeline> pipeline);
    
    /**
     * @brief Get the currently active graphics pipeline
     * @return Shared pointer to current pipeline, may be null
     */
    std::shared_ptr<Pipeline> getPipeline() const;

    // Subsystem access
    /**
     * @brief Get the shader manager instance
     * @return Shared pointer to shader manager
     */
    std::shared_ptr<ShaderManager> getShaderManager() const;

    /**
     * @brief Get the render context instance
     * @return Shared pointer to render context
     */
    std::shared_ptr<RenderContext> getRenderContext() const;

    // Window state management
    /**
     * @brief Check if window should close
     * @return true if window close was requested, false otherwise
     */
    bool shouldClose() const;
    
    /**
     * @brief Set window title
     * @param title New window title string
     */
    void setWindowTitle(const std::string& title);
    
    /**
     * @brief Get current window dimensions
     * @param width Reference to store window width
     * @param height Reference to store window height
     */
    void getWindowSize(int& width, int& height) const;

    // Callback registration
    /**
     * @brief Set custom framebuffer resize callback
     * @param callback Function to call when framebuffer is resized
     */
    void setFramebufferSizeCallback(std::function<void(int, int)> callback);
    
    /**
     * @brief Set custom keyboard input callback
     * @param callback Function to call on keyboard events
     */
    void setKeyCallback(std::function<void(int, int, int, int)> callback);

private:
    struct Impl;                        ///< Forward declaration for PIMPL idiom
    std::unique_ptr<Impl> impl;         ///< Private implementation pointer
}; 