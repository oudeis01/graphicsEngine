#pragma once
#include <string>
#include <GL/glew.h>

class Shader {
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    void use() const;
    GLuint id() const;
    void setFloat(const std::string& name, float value) const;
private:
    GLuint shaderID;
    std::string loadFile(const std::string& path);
    std::string preprocessShader(const std::string& source, const std::string& basePath);
    GLuint compileShader(const std::string& src, GLenum type);
};