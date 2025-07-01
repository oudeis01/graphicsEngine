#pragma once

#include <GL/glew.h>
#include <string>
#include <map>
#include <vector>
#include <memory>

namespace gfx {

/**
 * @brief LYGIA-based uber shader generation and management
 * 
 * Manages dynamic GLSL shader compilation, LYGIA module integration,
 * and hot-reloading for real-time shader development.
 */
class ShaderManager {
public:
    ShaderManager();
    ~ShaderManager();
    
    /**
     * @brief Initialize shader manager with LYGIA path
     * @param lygiaPath Path to LYGIA library directory
     * @return true if initialization successful, false otherwise
     */
    bool initialize(const std::string& lygiaPath);
    
    /**
     * @brief Shutdown and cleanup resources
     */
    void shutdown();
    
    /**
     * @brief Compile shader from node graph pipeline
     * @param nodeGraph Pipeline graph as string representation
     * @return Compiled shader program ID, 0 if failed
     */
    GLuint compileFromPipeline(const std::string& nodeGraph);
    
    /**
     * @brief Compile shader from GLSL source code
     * @param vertexSource Vertex shader source
     * @param fragmentSource Fragment shader source
     * @return Compiled shader program ID, 0 if failed
     */
    GLuint compileFromSource(const std::string& vertexSource, const std::string& fragmentSource);
    
    /**
     * @brief Hot-reload existing shader with new pipeline
     * @param programId Existing shader program ID
     * @param nodeGraph New pipeline graph
     * @return true if reload successful, false otherwise
     */
    bool hotReload(GLuint programId, const std::string& nodeGraph);
    
    /**
     * @brief Delete shader program
     * @param programId Shader program ID to delete
     */
    void deleteProgram(GLuint programId);
    
    /**
     * @brief Use shader program for rendering
     * @param programId Shader program ID
     */
    void useProgram(GLuint programId);
    
    /**
     * @brief Set uniform value in active shader
     * @param name Uniform name
     * @param value Uniform value
     */
    void setUniform(const std::string& name, float value);
    void setUniform(const std::string& name, int value);
    void setUniform(const std::string& name, float x, float y);
    void setUniform(const std::string& name, float x, float y, float z);
    void setUniform(const std::string& name, float x, float y, float z, float w);
    
    /**
     * @brief Get available LYGIA modules
     * @return Vector of available module names
     */
    std::vector<std::string> getAvailableModules() const;
    
    /**
     * @brief Check if shader manager is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const { return initialized_; }

private:
    /**
     * @brief Generate vertex shader from pipeline
     * @param nodeGraph Pipeline graph
     * @return Generated vertex shader source
     */
    std::string generateVertexShader(const std::string& nodeGraph);
    
    /**
     * @brief Generate fragment shader from pipeline
     * @param nodeGraph Pipeline graph
     * @return Generated fragment shader source
     */
    std::string generateFragmentShader(const std::string& nodeGraph);
    
    /**
     * @brief Compile individual shader
     * @param source Shader source code
     * @param type Shader type (GL_VERTEX_SHADER, GL_FRAGMENT_SHADER)
     * @return Compiled shader ID, 0 if failed
     */
    GLuint compileShader(const std::string& source, GLenum type);
    
    /**
     * @brief Link shader program
     * @param vertexShader Compiled vertex shader ID
     * @param fragmentShader Compiled fragment shader ID
     * @return Linked program ID, 0 if failed
     */
    GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader);
    
    /**
     * @brief Load LYGIA module content
     * @param moduleName Module name (e.g., "math/rotate2d")
     * @return Module source code
     */
    std::string loadLygiaModule(const std::string& moduleName);
    
    /**
     * @brief Scan for available LYGIA modules
     */
    void scanLygiaModules();
    
    /**
     * @brief Check for shader compilation errors
     * @param shader Shader ID
     * @param type Shader type name for error reporting
     * @return true if no errors, false if errors found
     */
    bool checkShaderErrors(GLuint shader, const std::string& type);
    
    /**
     * @brief Check for program linking errors
     * @param program Program ID
     * @return true if no errors, false if errors found
     */
    bool checkProgramErrors(GLuint program);
    
    std::string lygia_path_;                        ///< Path to LYGIA library
    std::map<std::string, std::string> modules_;    ///< Cached LYGIA modules
    std::vector<GLuint> active_programs_;           ///< Active shader programs
    GLuint current_program_;                        ///< Currently active program
    bool initialized_;                              ///< Initialization state
};

} // namespace gfx
