#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <regex>

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vSrc = loadFile(vertexPath);
    std::string fSrc = loadFile(fragmentPath);
    
    // Preprocess includes
    vSrc = preprocessShader(vSrc, std::filesystem::path(vertexPath).parent_path().string());
    fSrc = preprocessShader(fSrc, std::filesystem::path(fragmentPath).parent_path().string());
    
    GLuint vShader = compileShader(vSrc, GL_VERTEX_SHADER);
    GLuint fShader = compileShader(fSrc, GL_FRAGMENT_SHADER);

    shaderID = glCreateProgram();
    glAttachShader(shaderID, vShader);
    glAttachShader(shaderID, fShader);
    glLinkProgram(shaderID);

    GLint success;
    glGetProgramiv(shaderID, GL_LINK_STATUS, &success);
    if (!success) {
        char info[512];
        glGetProgramInfoLog(shaderID, 512, nullptr, info);
        std::cerr << "Shader link error: " << info << std::endl;
    }
    glDeleteShader(vShader);
    glDeleteShader(fShader);
}

void Shader::use() const { glUseProgram(shaderID); }
GLuint Shader::id() const { return shaderID; }

void Shader::setFloat(const std::string& name, float value) const {
    GLint loc = glGetUniformLocation(shaderID, name.c_str());
    if (loc != -1) glUniform1f(loc, value);
}

std::string Shader::loadFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << path << std::endl;
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

std::string Shader::preprocessShader(const std::string& source, const std::string& basePath) {
    std::string result = source;
    std::regex includeRegex("#include\\s+\"([^\"]+)\"");
    std::smatch match;
    
    std::cout << "Preprocessing shader in path: " << basePath << std::endl;
    
    while (std::regex_search(result, match, includeRegex)) {
        std::string includePath = basePath + "/" + match[1].str();
        std::cout << "Including file: " << includePath << std::endl;
        
        std::string includeContent = loadFile(includePath);
        if (includeContent.empty()) {
            std::cerr << "Warning: Include file is empty or not found: " << includePath << std::endl;
        } else {
            std::cout << "Successfully loaded include file: " << includePath << " (" << includeContent.length() << " chars)" << std::endl;
        }
        
        // Recursively process includes in the included file
        includeContent = preprocessShader(includeContent, std::filesystem::path(includePath).parent_path().string());
        
        // Replace the #include directive with the file content
        result = std::regex_replace(result, includeRegex, includeContent, std::regex_constants::format_first_only);
    }
    
    return result;
}

GLuint Shader::compileShader(const std::string& src, GLenum type) {
    GLuint shader = glCreateShader(type);
    const char* code = src.c_str();
    glShaderSource(shader, 1, &code, nullptr);
    glCompileShader(shader);
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info[512];
        glGetShaderInfoLog(shader, 512, nullptr, info);
        std::cerr << "Shader compile error (" << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << "): " << info << std::endl;
        std::cerr << "Source code:\n" << src << std::endl;
    } else {
        std::cout << "Shader compiled successfully (" << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << ")" << std::endl;
    }
    return shader;
}