#include "Shader.h"
#include "PipelineParser.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>

int main() {
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(800, 600, "Shader DSL", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW init failed" << std::endl;
        return -1;
    }

    // Print OpenGL version info for debugging
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    // Create and bind VAO (CRITICAL - required in Core Profile)
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error after VAO creation: " << error << std::endl;
    }

    Shader shader("./shaders/passthrough.vert", "./shaders/default.frag");
    PipelineParser pipeline("./pipeline.txt");

    auto lastWriteTime = std::filesystem::last_write_time("./pipeline.txt");

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        auto currentWriteTime = std::filesystem::last_write_time("./pipeline.txt");
        if (currentWriteTime != lastWriteTime) {
            pipeline.reload();
            lastWriteTime = currentWriteTime;
        }

        shader.use();
        pipeline.update(shader);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glfwSwapBuffers(window);
        glfwPollEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
