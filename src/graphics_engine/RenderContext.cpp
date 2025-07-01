#include "RenderContext.h"
#include <iostream>

namespace gfx {

RenderContext::RenderContext() 
    : window_(nullptr)
    , window_width_(0)
    , window_height_(0)
    , initialized_(false) {
}

RenderContext::~RenderContext() {
    shutdown();
}

bool RenderContext::initialize(int width, int height, const std::string& title) {
    if (initialized_) {
        return true;
    }
    
    // Set GLFW error callback
    glfwSetErrorCallback(glfwErrorCallback);
    
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    // Set OpenGL context hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on macOS
    
    // Create window
    window_ = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window_) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    
    // Make context current
    glfwMakeContextCurrent(window_);
    
    // Set framebuffer resize callback
    glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);
    
    // Initialize GLEW
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(glewError) << std::endl;
        glfwDestroyWindow(window_);
        glfwTerminate();
        window_ = nullptr;
        return false;
    }
    
    // Check OpenGL version
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    
    // Store window dimensions
    window_width_ = width;
    window_height_ = height;
    
    // Set initial viewport
    setViewport(width, height);
    
    // Enable vsync
    glfwSwapInterval(1);
    
    initialized_ = true;
    return true;
}

void RenderContext::shutdown() {
    if (window_) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }
    
    if (initialized_) {
        glfwTerminate();
        initialized_ = false;
    }
}

bool RenderContext::shouldClose() const {
    return window_ ? glfwWindowShouldClose(window_) : true;
}

void RenderContext::swapBuffers() {
    if (window_) {
        glfwSwapBuffers(window_);
    }
}

void RenderContext::pollEvents() {
    glfwPollEvents();
}

void RenderContext::clear(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderContext::setViewport(int width, int height) {
    glViewport(0, 0, width, height);
    window_width_ = width;
    window_height_ = height;
}

void RenderContext::getWindowSize(int& width, int& height) const {
    width = window_width_;
    height = window_height_;
}

void RenderContext::glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

void RenderContext::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

} // namespace gfx
