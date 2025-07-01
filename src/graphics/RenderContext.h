#pragma once

#include <GL/glew.h>
#include <memory>

/**
 * @brief OpenGL rendering context management class
 * @details Manages OpenGL state, viewport settings, framebuffers, and basic geometry.
 *          Uses PIMPL pattern to hide implementation details and reduce compilation dependencies.
 *          Supports OpenGL 4.1 Core Profile for cross-platform compatibility including macOS.
 * 
 * Key responsibilities:
 * - OpenGL context state management
 * - Viewport and clear color configuration
 * - Framebuffer object lifecycle management
 * - Fullscreen quad geometry for shader effects
 * - Timing utilities for animations
 */
class RenderContext {
public:
    /**
     * @brief Default constructor
     */
    RenderContext();
    
    /**
     * @brief Destructor - ensures proper cleanup
     */
    ~RenderContext();

    /**
     * @brief Initialize the render context
     * @param width Initial viewport width in pixels
     * @param height Initial viewport height in pixels
     * @return true if initialization succeeded, false otherwise
     */
    bool initialize(int width, int height);
    
    /**
     * @brief Clean up OpenGL resources
     */
    void cleanup();

    /**
     * @brief Set viewport dimensions
     * @param width Viewport width in pixels
     * @param height Viewport height in pixels
     */
    void setViewport(int width, int height);
    
    /**
     * @brief Get current viewport dimensions
     * @param width Reference to store viewport width
     * @param height Reference to store viewport height
     */
    void getViewport(int& width, int& height) const;
    
    /**
     * @brief Get current viewport width
     * @return Viewport width in pixels
     */
    int getViewportWidth() const;
    
    /**
     * @brief Get current viewport height
     * @return Viewport height in pixels
     */
    int getViewportHeight() const;
    
    /**
     * @brief Set frame buffer clear color
     * @param r Red component (0.0-1.0)
     * @param g Green component (0.0-1.0)
     * @param b Blue component (0.0-1.0)
     * @param a Alpha component (0.0-1.0), defaults to 1.0
     */
    void setClearColor(float r, float g, float b, float a = 1.0f);
    
    /**
     * @brief Clear the frame buffer
     */
    void clear();

    /**
     * @brief Render a fullscreen quad for shader effects
     */
    void renderFullscreenQuad();

    /**
     * @brief Create a framebuffer object with color attachment
     * @param width Framebuffer width in pixels
     * @param height Framebuffer height in pixels
     * @return OpenGL framebuffer object ID, or 0 if creation failed
     */
    GLuint createFramebuffer(int width, int height);
    
    /**
     * @brief Bind a framebuffer for rendering
     * @param fbo Framebuffer object ID (0 for default framebuffer)
     */
    void bindFramebuffer(GLuint fbo);
    
    /**
     * @brief Delete a framebuffer object
     * @param fbo Framebuffer object ID to delete
     */
    void deleteFramebuffer(GLuint fbo);

    /**
     * @brief Get elapsed time since initialization
     * @return Time in seconds
     */
    float getTime() const;

private:
    struct Impl;                        ///< Forward declaration for PIMPL pattern
    std::unique_ptr<Impl> impl;         ///< PIMPL implementation pointer
};
