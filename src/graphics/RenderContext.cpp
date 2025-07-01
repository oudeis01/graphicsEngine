#include "RenderContext.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

/**
 * @brief Implementation details for RenderContext using PIMPL pattern
 */
struct RenderContext::Impl {
    int viewportWidth = 0;      ///< Current viewport width in pixels
    int viewportHeight = 0;     ///< Current viewport height in pixels
    
    float clearColor[4] = {0.1f, 0.1f, 0.1f, 1.0f};  ///< RGBA clear color values
    
    GLuint quadVAO = 0;         ///< Vertex Array Object for fullscreen quad
    GLuint quadVBO = 0;         ///< Vertex Buffer Object for fullscreen quad
    
    double startTime = 0.0;     ///< Application start time for relative timing
};

/**
 * @brief Default constructor
 * @details Initializes the PIMPL implementation object
 */
RenderContext::RenderContext() : impl(std::make_unique<Impl>()) {}

/**
 * @brief Destructor
 * @details Ensures proper cleanup of OpenGL resources
 */
RenderContext::~RenderContext() {
    cleanup();
}

/**
 * @brief Initialize the render context with specified dimensions
 * @param width Viewport width in pixels
 * @param height Viewport height in pixels
 * @return true if initialization succeeded, false otherwise
 */
bool RenderContext::initialize(int width, int height) {
    impl->startTime = glfwGetTime();
    
    setViewport(width, height);
    
    // Setup fullscreen quad vertex data
    float quadVertices[] = {
        // Position     // Texture coordinates
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &impl->quadVAO);
    glGenBuffers(1, &impl->quadVBO);
    
    glBindVertexArray(impl->quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, impl->quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    
    // Position attribute (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    
    // Texture coordinate attribute (location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    
    glBindVertexArray(0);

    // OpenGL default settings
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    std::cout << "RenderContext initialized successfully" << std::endl;
    return true;
}

/**
 * @brief Clean up OpenGL resources
 * @details Deletes VAO and VBO objects if they exist
 */
void RenderContext::cleanup() {
    if (impl->quadVBO != 0) {
        glDeleteBuffers(1, &impl->quadVBO);
        impl->quadVBO = 0;
    }
    if (impl->quadVAO != 0) {
        glDeleteVertexArrays(1, &impl->quadVAO);
        impl->quadVAO = 0;
    }
}

/**
 * @brief Set viewport dimensions and update OpenGL viewport
 * @param width New viewport width in pixels
 * @param height New viewport height in pixels
 */
void RenderContext::setViewport(int width, int height) {
    impl->viewportWidth = width;
    impl->viewportHeight = height;
    glViewport(0, 0, width, height);
}

/**
 * @brief Get current viewport dimensions
 * @param width Reference to store viewport width
 * @param height Reference to store viewport height
 */
void RenderContext::getViewport(int& width, int& height) const {
    width = impl->viewportWidth;
    height = impl->viewportHeight;
}

/**
 * @brief Get current viewport width
 * @return Viewport width in pixels
 */
int RenderContext::getViewportWidth() const {
    return impl->viewportWidth;
}

/**
 * @brief Get current viewport height
 * @return Viewport height in pixels
 */
int RenderContext::getViewportHeight() const {
    return impl->viewportHeight;
}

/**
 * @brief Set clear color for frame buffer clearing
 * @param r Red component (0.0-1.0)
 * @param g Green component (0.0-1.0)
 * @param b Blue component (0.0-1.0)
 * @param a Alpha component (0.0-1.0)
 */
void RenderContext::setClearColor(float r, float g, float b, float a) {
    impl->clearColor[0] = r;
    impl->clearColor[1] = g;
    impl->clearColor[2] = b;
    impl->clearColor[3] = a;
}

/**
 * @brief Clear the frame buffer with current clear color
 * @details Clears both color and depth buffers
 */
void RenderContext::clear() {
    glClearColor(impl->clearColor[0], impl->clearColor[1], 
                 impl->clearColor[2], impl->clearColor[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

/**
 * @brief Render a fullscreen quad for shader effects
 * @details Uses the pre-configured VAO to draw two triangles covering the screen
 */
void RenderContext::renderFullscreenQuad() {
    glBindVertexArray(impl->quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

/**
 * @brief Create a framebuffer object with color attachment
 * @param width Framebuffer width in pixels
 * @param height Framebuffer height in pixels
 * @return OpenGL framebuffer object ID, or 0 if creation failed
 */
GLuint RenderContext::createFramebuffer(int width, int height) {
    GLuint fbo, texture;
    
    // Create framebuffer object
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    
    // Create color texture attachment
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    
    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer is not complete!" << std::endl;
        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(1, &texture);
        return 0;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return fbo;
}

/**
 * @brief Bind a framebuffer for rendering
 * @param fbo Framebuffer object ID to bind (0 for default framebuffer)
 */
void RenderContext::bindFramebuffer(GLuint fbo) {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

/**
 * @brief Delete a framebuffer object
 * @param fbo Framebuffer object ID to delete
 */
void RenderContext::deleteFramebuffer(GLuint fbo) {
    if (fbo != 0) {
        glDeleteFramebuffers(1, &fbo);
    }
}

/**
 * @brief Get elapsed time since context initialization
 * @return Time in seconds as floating point value
 */
float RenderContext::getTime() const {
    return static_cast<float>(glfwGetTime() - impl->startTime);
}
