#include "ShaderManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

namespace gfx {

ShaderManager::ShaderManager()
    : current_program_(0)
    , initialized_(false) {
}

ShaderManager::~ShaderManager() {
    shutdown();
}

bool ShaderManager::initialize(const std::string& lygiaPath) {
    if (initialized_) {
        return true;
    }
    
    lygia_path_ = lygiaPath;
    
    // Check if LYGIA directory exists
    if (!std::filesystem::exists(lygia_path_)) {
        std::cerr << "LYGIA directory not found: " << lygia_path_ << std::endl;
        return false;
    }
    
    // Scan for available modules
    scanLygiaModules();
    
    std::cout << "ShaderManager initialized with " << modules_.size() << " LYGIA modules" << std::endl;
    
    initialized_ = true;
    return true;
}

void ShaderManager::shutdown() {
    // Delete all active programs
    for (GLuint program : active_programs_) {
        if (glIsProgram(program)) {
            glDeleteProgram(program);
        }
    }
    active_programs_.clear();
    
    // Clear modules cache
    modules_.clear();
    
    current_program_ = 0;
    initialized_ = false;
}

GLuint ShaderManager::compileFromPipeline(const std::string& nodeGraph) {
    if (!initialized_) {
        std::cerr << "ShaderManager not initialized" << std::endl;
        return 0;
    }
    
    // Generate shaders from pipeline
    std::string vertexSource = generateVertexShader(nodeGraph);
    std::string fragmentSource = generateFragmentShader(nodeGraph);
    
    return compileFromSource(vertexSource, fragmentSource);
}

GLuint ShaderManager::compileFromSource(const std::string& vertexSource, const std::string& fragmentSource) {
    // Compile vertex shader
    GLuint vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    if (vertexShader == 0) {
        return 0;
    }
    
    // Compile fragment shader
    GLuint fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        return 0;
    }
    
    // Link program
    GLuint program = linkProgram(vertexShader, fragmentShader);
    
    // Clean up individual shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    if (program != 0) {
        active_programs_.push_back(program);
    }
    
    return program;
}

bool ShaderManager::hotReload(GLuint programId, const std::string& nodeGraph) {
    if (!initialized_ || !glIsProgram(programId)) {
        return false;
    }
    
    // Compile new program
    GLuint newProgram = compileFromPipeline(nodeGraph);
    if (newProgram == 0) {
        return false;
    }
    
    // Replace old program
    auto it = std::find(active_programs_.begin(), active_programs_.end(), programId);
    if (it != active_programs_.end()) {
        glDeleteProgram(programId);
        *it = newProgram;
        
        if (current_program_ == programId) {
            current_program_ = newProgram;
            glUseProgram(newProgram);
        }
        
        return true;
    }
    
    return false;
}

void ShaderManager::deleteProgram(GLuint programId) {
    if (glIsProgram(programId)) {
        glDeleteProgram(programId);
        
        auto it = std::find(active_programs_.begin(), active_programs_.end(), programId);
        if (it != active_programs_.end()) {
            active_programs_.erase(it);
        }
        
        if (current_program_ == programId) {
            current_program_ = 0;
        }
    }
}

void ShaderManager::useProgram(GLuint programId) {
    if (glIsProgram(programId)) {
        current_program_ = programId;
        glUseProgram(programId);
    }
}

void ShaderManager::setUniform(const std::string& name, float value) {
    if (current_program_ != 0) {
        GLint location = glGetUniformLocation(current_program_, name.c_str());
        if (location != -1) {
            glUniform1f(location, value);
        }
    }
}

void ShaderManager::setUniform(const std::string& name, int value) {
    if (current_program_ != 0) {
        GLint location = glGetUniformLocation(current_program_, name.c_str());
        if (location != -1) {
            glUniform1i(location, value);
        }
    }
}

void ShaderManager::setUniform(const std::string& name, float x, float y) {
    if (current_program_ != 0) {
        GLint location = glGetUniformLocation(current_program_, name.c_str());
        if (location != -1) {
            glUniform2f(location, x, y);
        }
    }
}

void ShaderManager::setUniform(const std::string& name, float x, float y, float z) {
    if (current_program_ != 0) {
        GLint location = glGetUniformLocation(current_program_, name.c_str());
        if (location != -1) {
            glUniform3f(location, x, y, z);
        }
    }
}

void ShaderManager::setUniform(const std::string& name, float x, float y, float z, float w) {
    if (current_program_ != 0) {
        GLint location = glGetUniformLocation(current_program_, name.c_str());
        if (location != -1) {
            glUniform4f(location, x, y, z, w);
        }
    }
}

std::vector<std::string> ShaderManager::getAvailableModules() const {
    std::vector<std::string> moduleNames;
    moduleNames.reserve(modules_.size());
    
    for (const auto& [name, content] : modules_) {
        moduleNames.push_back(name);
    }
    
    return moduleNames;
}

std::string ShaderManager::generateVertexShader(const std::string& nodeGraph) {
    // Basic vertex shader template
    // TODO: Implement actual pipeline-to-shader generation
    return R"(
#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main() {
    gl_Position = vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}
)";
}

std::string ShaderManager::generateFragmentShader(const std::string& nodeGraph) {
    // Basic fragment shader template
    // TODO: Implement actual pipeline-to-shader generation using LYGIA modules
    std::stringstream ss;
    ss << "#version 410 core\n";
    ss << "\n";
    ss << "in vec2 TexCoord;\n";
    ss << "out vec4 FragColor;\n";
    ss << "\n";
    ss << "uniform float u_time;\n";
    ss << "uniform vec2 u_resolution;\n";
    ss << "\n";
    
    // Include basic LYGIA modules if available
    if (modules_.find("math/rotate2d") != modules_.end()) {
        ss << "// LYGIA: math/rotate2d\n";
        ss << modules_.at("math/rotate2d") << "\n";
    }
    
    ss << "void main() {\n";
    ss << "    vec2 uv = TexCoord;\n";
    ss << "    vec3 color = vec3(uv, 0.5 + 0.5 * sin(u_time));\n";
    ss << "    FragColor = vec4(color, 1.0);\n";
    ss << "}\n";
    
    return ss.str();
}

GLuint ShaderManager::compileShader(const std::string& source, GLenum type) {
    GLuint shader = glCreateShader(type);
    
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    
    if (!checkShaderErrors(shader, type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT")) {
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

GLuint ShaderManager::linkProgram(GLuint vertexShader, GLuint fragmentShader) {
    GLuint program = glCreateProgram();
    
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    if (!checkProgramErrors(program)) {
        glDeleteProgram(program);
        return 0;
    }
    
    return program;
}

std::string ShaderManager::loadLygiaModule(const std::string& moduleName) {
    std::string filePath = lygia_path_ + "/" + moduleName + ".glsl";
    
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void ShaderManager::scanLygiaModules() {
    modules_.clear();
    
    if (!std::filesystem::exists(lygia_path_)) {
        return;
    }
    
    // Scan for .glsl files recursively
    for (const auto& entry : std::filesystem::recursive_directory_iterator(lygia_path_)) {
        if (entry.is_regular_file() && entry.path().extension() == ".glsl") {
            std::string relativePath = std::filesystem::relative(entry.path(), lygia_path_);
            
            // Remove .glsl extension for module name
            std::string moduleName = relativePath.substr(0, relativePath.length() - 5);
            
            // Load module content
            std::string content = loadLygiaModule(moduleName);
            if (!content.empty()) {
                modules_[moduleName] = content;
            }
        }
    }
}

bool ShaderManager::checkShaderErrors(GLuint shader, const std::string& type) {
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    
    if (!success) {
        GLchar infoLog[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
        std::cerr << "Shader compilation error (" << type << "): " << infoLog << std::endl;
        return false;
    }
    
    return true;
}

bool ShaderManager::checkProgramErrors(GLuint program) {
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    
    if (!success) {
        GLchar infoLog[1024];
        glGetProgramInfoLog(program, 1024, nullptr, infoLog);
        std::cerr << "Program linking error: " << infoLog << std::endl;
        return false;
    }
    
    return true;
}

} // namespace gfx
