#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <string>

namespace gfx {

/**
 * @brief OpenGL context management and basic rendering state
 * 
 * Manages GLFW window creation, OpenGL context initialization,
 * and basic rendering operations for the Graphics Engine.
 */
class RenderContext {
public:
    RenderContext();
    ~RenderContext();
    
    /**
     * @brief Initialize GLFW window and OpenGL context
     * @param width Window width in pixels
     * @param height Window height in pixels
     * @param title Window title
     * @return true if initialization successful, false otherwise
     */
    bool initialize(int width, int height, const std::string& title);
    
    /**
     * @brief Clean up OpenGL context and close window
     */
    void shutdown();
    
    /**
     * @brief Check if window should close
     * @return true if window close requested, false otherwise
     */
    bool shouldClose() const;
    
    /**
     * @brief Swap front and back buffers
     */
    void swapBuffers();
    
    /**
     * @brief Poll GLFW events
     */
    void pollEvents();
    
    /**
     * @brief Clear the framebuffer
     * @param r Red component (0.0-1.0)
     * @param g Green component (0.0-1.0)
     * @param b Blue component (0.0-1.0)
     * @param a Alpha component (0.0-1.0)
     */
    void clear(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);
    
    /**
     * @brief Set viewport dimensions
     * @param width Viewport width
     * @param height Viewport height
     */
    void setViewport(int width, int height);
    
    /**
     * @brief Get current window dimensions
     * @param width Output parameter for width
     * @param height Output parameter for height
     */
    void getWindowSize(int& width, int& height) const;
    
    /**
     * @brief Check if context is valid
     * @return true if context initialized, false otherwise
     */
    bool isValid() const { return window_ != nullptr; }
    
    /**
     * @brief Get GLFW window handle
     * @return GLFWwindow pointer
     */
    GLFWwindow* getWindow() const { return window_; }

private:
    /**
     * @brief GLFW error callback
     * @param error Error code
     * @param description Error description
     */
    static void glfwErrorCallback(int error, const char* description);
    
    /**
     * @brief Window resize callback
     * @param window GLFW window handle
     * @param width New window width
     * @param height New window height
     */
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    
    GLFWwindow* window_;        ///< GLFW window handle
    int window_width_;          ///< Current window width
    int window_height_;         ///< Current window height
    bool initialized_;          ///< Initialization state
};

} // namespace gfx
