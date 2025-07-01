#include "Pipeline.h"
#include "ShaderManager.h"
#include <GL/glew.h>
#include <iostream>
#include <sstream>

namespace gfx {

Pipeline::Pipeline()
    : shader_program_(0)
    , vao_(0)
    , vbo_(0)
    , ebo_(0)
    , initialized_(false)
    , total_time_(0.0f) {
}

Pipeline::~Pipeline() {
    shutdown();
}

bool Pipeline::initialize(std::shared_ptr<ShaderManager> shaderManager) {
    if (initialized_) {
        return true;
    }
    
    if (!shaderManager || !shaderManager->isInitialized()) {
        std::cerr << "Invalid shader manager provided to Pipeline" << std::endl;
        return false;
    }
    
    shader_manager_ = shaderManager;
    
    // Setup default rendering quad
    setupQuad();
    
    // Generate initial shader
    generateShader();
    
    initialized_ = true;
    std::cout << "Pipeline initialized successfully" << std::endl;
    return true;
}

void Pipeline::shutdown() {
    cleanupQuad();
    
    if (shader_program_ != 0) {
        shader_manager_->deleteProgram(shader_program_);
        shader_program_ = 0;
    }
    
    shader_manager_.reset();
    initialized_ = false;
}

bool Pipeline::updateFromNodeGraph(const NodeGraph& nodeGraph) {
    if (!initialized_) {
        return false;
    }
    
    node_graph_ = nodeGraph;
    return generateShader();
}

bool Pipeline::updateFromString(const std::string& pipelineData) {
    if (!initialized_) {
        return false;
    }
    
    // TODO: Parse pipeline data string into node graph
    // For now, use a simple approach
    std::cout << "Updating pipeline from string: " << pipelineData << std::endl;
    
    return generateShader();
}

void Pipeline::render(float deltaTime) {
    if (!isReady()) {
        return;
    }
    
    total_time_ += deltaTime;
    
    // Use current shader program
    shader_manager_->useProgram(shader_program_);
    
    // Update uniforms
    updateUniforms(deltaTime);
    
    // Render quad
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

bool Pipeline::setParameter(int nodeId, const std::string& paramName, const std::string& value) {
    if (!initialized_) {
        return false;
    }
    
    // TODO: Update node parameter in graph and regenerate shader if needed
    std::cout << "Setting parameter: Node " << nodeId << ", " << paramName << " = " << value << std::endl;
    
    // For now, just try to set as uniform if shader is active
    if (shader_program_ != 0) {
        try {
            float floatValue = std::stof(value);
            shader_manager_->setUniform(paramName, floatValue);
            return true;
        } catch (const std::exception& e) {
            // Try as int
            try {
                int intValue = std::stoi(value);
                shader_manager_->setUniform(paramName, intValue);
                return true;
            } catch (const std::exception& e) {
                // Could not parse as number
                return false;
            }
        }
    }
    
    return false;
}

std::string Pipeline::getPipelineString() const {
    // TODO: Serialize current node graph to string
    return "DefaultPipeline";
}

bool Pipeline::isReady() const {
    return initialized_ && shader_program_ != 0 && vao_ != 0;
}

bool Pipeline::generateShader() {
    if (!shader_manager_) {
        return false;
    }
    
    // Convert node graph to pipeline string for shader generation
    std::string pipelineString = getPipelineString();
    
    // Generate new shader
    unsigned int newProgram = shader_manager_->compileFromPipeline(pipelineString);
    if (newProgram == 0) {
        std::cerr << "Failed to generate shader from pipeline" << std::endl;
        return false;
    }
    
    // Delete old shader if exists
    if (shader_program_ != 0) {
        shader_manager_->deleteProgram(shader_program_);
    }
    
    shader_program_ = newProgram;
    std::cout << "Generated new shader program: " << shader_program_ << std::endl;
    return true;
}

void Pipeline::setupQuad() {
    // Fullscreen quad vertices
    float vertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,  // top left
        -1.0f, -1.0f,  0.0f, 0.0f,  // bottom left
         1.0f, -1.0f,  1.0f, 0.0f,  // bottom right
         1.0f,  1.0f,  1.0f, 1.0f   // top right
    };
    
    unsigned int indices[] = {
        0, 1, 2,  // first triangle
        0, 2, 3   // second triangle
    };
    
    // Generate VAO, VBO, EBO
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);
    
    // Bind VAO
    glBindVertexArray(vao_);
    
    // Bind and set VBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Bind and set EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Unbind VAO
    glBindVertexArray(0);
}

void Pipeline::cleanupQuad() {
    if (vao_ != 0) {
        glDeleteVertexArrays(1, &vao_);
        vao_ = 0;
    }
    if (vbo_ != 0) {
        glDeleteBuffers(1, &vbo_);
        vbo_ = 0;
    }
    if (ebo_ != 0) {
        glDeleteBuffers(1, &ebo_);
        ebo_ = 0;
    }
}

void Pipeline::updateUniforms(float deltaTime) {
    if (shader_program_ == 0) {
        return;
    }
    
    // Update standard uniforms
    shader_manager_->setUniform("u_time", total_time_);
    shader_manager_->setUniform("u_deltaTime", deltaTime);
    shader_manager_->setUniform("u_resolution", 800.0f, 600.0f); // TODO: Get actual resolution
}

} // namespace gfx
