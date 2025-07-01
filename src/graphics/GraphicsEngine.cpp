#include "GraphicsEngine.h"
#include "RenderContext.h"
#include "ShaderManager.h" 
#include "Pipeline.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>

namespace {
    /**
     * @brief GLFW error callback function
     * @param error Error code from GLFW
     * @param description Human-readable error description
     */
    void errorCallback(int error, const char* description) {
        std::cerr << "GLFW Error " << error << ": " << description << std::endl;
    }

    /**
     * @brief Framebuffer resize callback for automatic viewport adjustment
     * @param window GLFW window that was resized
     * @param width New framebuffer width
     * @param height New framebuffer height
     */
    void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
        auto* engine = static_cast<GraphicsEngine*>(glfwGetWindowUserPointer(window));
        if (engine && engine->getRenderContext()) {
            engine->getRenderContext()->setViewport(width, height);
        }
    }
}

/**
 * @brief Internal implementation structure for GraphicsEngine
 * Encapsulates private data members using PIMPL idiom
 */
struct GraphicsEngine::Impl {
    GLFWwindow* window = nullptr;                           ///< GLFW window handle
    bool shouldStop = false;                                ///< Flag to request main loop termination
    
    std::shared_ptr<RenderContext> renderContext;          ///< OpenGL rendering context manager
    std::shared_ptr<ShaderManager> shaderManager;          ///< GLSL shader compilation and management
    std::shared_ptr<Pipeline> currentPipeline;             ///< Active graphics pipeline
    
    // User-defined callback functions
    std::function<void(int, int)> framebufferCallback;     ///< Custom framebuffer resize callback
    std::function<void(int, int, int, int)> keyCallback;   ///< Custom keyboard input callback
};

/**
 * @brief Default constructor
 * Initializes PIMPL structure
 */
GraphicsEngine::GraphicsEngine() : impl(std::make_unique<Impl>()) {}

/**
 * @brief Destructor
 * Ensures proper cleanup of resources
 */
GraphicsEngine::~GraphicsEngine() {
    cleanup();
}

/**
 * @brief Initialize the graphics engine with window and OpenGL context
 * @param width Initial window width in pixels
 * @param height Initial window height in pixels
 * @param title Window title string
 * @return true if initialization successful, false otherwise
 */
bool GraphicsEngine::initialize(int width, int height, const char* title) {
    // Setup GLFW error callback
    glfwSetErrorCallback(errorCallback);

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // OpenGL 4.1 Core Profile configuration (macOS compatible)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    
    impl->window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!impl->window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    // Store engine instance pointer for callbacks
    glfwSetWindowUserPointer(impl->window, this);
    
    // Make OpenGL context current
    glfwMakeContextCurrent(impl->window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize GLEW for OpenGL function loading
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(err) << std::endl;
        return false;
    }

    // Check for OpenGL errors after initialization
    GLenum glErr = glGetError();
    if (glErr != GL_NO_ERROR) {
        std::cerr << "OpenGL error after initialization: " << glErr << std::endl;
    }

    // Initialize subsystems
    impl->renderContext = std::make_shared<RenderContext>();
    if (!impl->renderContext->initialize(width, height)) {
        std::cerr << "Failed to initialize RenderContext" << std::endl;
        return false;
    }

    impl->shaderManager = std::make_shared<ShaderManager>();
    if (!impl->shaderManager->initialize()) {
        std::cerr << "Failed to initialize ShaderManager" << std::endl;
        return false;
    }

    // Create default pipeline
    impl->currentPipeline = std::make_shared<Pipeline>();
    if (!impl->currentPipeline->initialize(impl->renderContext.get(), impl->shaderManager.get())) {
        std::cerr << "Failed to initialize default Pipeline" << std::endl;
        return false;
    }
    
    // Create a simple default pipeline for testing
    int noiseNode = impl->currentPipeline->addNode("noise");
    if (noiseNode >= 0) {
        impl->currentPipeline->setOutput(noiseNode);
        if (!impl->currentPipeline->compile()) {
            std::cerr << "Warning: Failed to compile default pipeline" << std::endl;
        }
    }

    // Setup window callbacks
    glfwSetFramebufferSizeCallback(impl->window, framebufferSizeCallback);

    std::cout << "Graphics Engine initialized successfully" << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    
    return true;
}

/**
 * @brief Main rendering loop - continues until window should close
 * Handles window events and renders frames continuously
 */
void GraphicsEngine::mainLoop() {
    while (!glfwWindowShouldClose(impl->window) && !impl->shouldStop) {
        renderFrame();
        glfwSwapBuffers(impl->window);
        glfwPollEvents();
    }
}

/**
 * @brief Render a single frame
 * Clears the screen and executes current pipeline rendering
 */
void GraphicsEngine::renderFrame() {
    if (!impl->renderContext || !impl->shaderManager) {
        return;
    }

    // Clear screen
    impl->renderContext->clear();

    // Render current pipeline if available
    if (impl->currentPipeline) {
        impl->currentPipeline->render(*impl->renderContext, *impl->shaderManager);
    }
}

/**
 * @brief Clean up all graphics resources and terminate GLFW
 * Should be called before application exit
 */
void GraphicsEngine::cleanup() {
    impl->currentPipeline.reset();
    impl->shaderManager.reset();
    impl->renderContext.reset();

    if (impl->window) {
        glfwDestroyWindow(impl->window);
        impl->window = nullptr;
    }
    glfwTerminate();
}

/**
 * @brief Request main loop termination
 * Sets internal flag to exit main loop gracefully
 */
void GraphicsEngine::stop() {
    impl->shouldStop = true;
}

/**
 * @brief Set the active graphics pipeline
 * @param pipeline Shared pointer to the pipeline to activate
 */
void GraphicsEngine::setPipeline(std::shared_ptr<Pipeline> pipeline) {
    impl->currentPipeline = pipeline;
}

/**
 * @brief Get the currently active graphics pipeline
 * @return Shared pointer to current pipeline, may be null
 */
std::shared_ptr<Pipeline> GraphicsEngine::getPipeline() const {
    return impl->currentPipeline;
}

/**
 * @brief Get the shader manager instance
 * @return Shared pointer to shader manager
 */
std::shared_ptr<ShaderManager> GraphicsEngine::getShaderManager() const {
    return impl->shaderManager;
}

/**
 * @brief Get the render context instance
 * @return Shared pointer to render context
 */
std::shared_ptr<RenderContext> GraphicsEngine::getRenderContext() const {
    return impl->renderContext;
}

/**
 * @brief Check if window should close
 * @return true if window close was requested, false otherwise
 */
bool GraphicsEngine::shouldClose() const {
    return impl->window ? glfwWindowShouldClose(impl->window) : true;
}

/**
 * @brief Set window title
 * @param title New window title string
 */
void GraphicsEngine::setWindowTitle(const std::string& title) {
    if (impl->window) {
        glfwSetWindowTitle(impl->window, title.c_str());
    }
}

/**
 * @brief Get current window dimensions
 * @param width Reference to store window width
 * @param height Reference to store window height
 */
void GraphicsEngine::getWindowSize(int& width, int& height) const {
    if (impl->window) {
        glfwGetWindowSize(impl->window, &width, &height);
    } else {
        width = height = 0;
    }
}

/**
 * @brief Set custom framebuffer resize callback
 * @param callback Function to call when framebuffer is resized
 */
void GraphicsEngine::setFramebufferSizeCallback(std::function<void(int, int)> callback) {
    impl->framebufferCallback = callback;
}

/**
 * @brief Set custom keyboard input callback
 * @param callback Function to call on keyboard events
 */
void GraphicsEngine::setKeyCallback(std::function<void(int, int, int, int)> callback) {
    impl->keyCallback = callback;
} 