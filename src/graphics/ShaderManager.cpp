#include "ShaderManager.h"
#include "GeneratorModules.h"
#include "OperatorModules.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <algorithm>
#include <thread>
#include <filesystem>
#include <random>
#include <chrono>
#include <sys/stat.h>

/**
 * @brief Internal implementation structure for ShaderManager
 * Encapsulates all private data and functionality using PIMPL idiom
 */
struct ShaderManager::Impl {
    GLuint currentShader = 0;                                   ///< Currently bound shader program
    GLuint defaultShader = 0;                                   ///< Fallback default shader program
    GLuint fullscreenQuadVAO = 0;                              ///< VAO for fullscreen quad rendering
    
    // Caching systems
    std::unordered_map<std::string, GLuint> shaderCache;        ///< Compiled shader cache
    std::unordered_map<std::string, std::string> moduleCache;   ///< LYGIA module cache
    std::unordered_map<GLuint, std::string> programSources;    ///< Source code for programs
    std::unordered_map<int, RenderTarget> nodeRenderTargets;   ///< FBO render targets per node
    
    // Hot-reload system
    bool hotReloadEnabled = false;                              ///< Hot-reload enable flag
    std::unordered_map<std::string, std::filesystem::file_time_type> fileTimestamps; ///< File modification tracking
    std::function<void(GLuint, const std::string&)> reloadCallback; ///< Reload event callback
    
    // Path configuration
    std::string lygiaPath = "external/lygia";                   ///< LYGIA library path
    std::string shaderPath = "shaders";                         ///< Shader files path
    
    // Statistics
    int compilationCount = 0;                                   ///< Total compilations performed
    int cacheHits = 0;                                         ///< Cache hit counter
    int hotReloads = 0;                                        ///< Hot-reload counter
    
    // Available LYGIA modules discovery
    std::vector<std::string> availableModules;                 ///< List of discovered modules
    
    // Module systems
    std::unique_ptr<GeneratorModules> generatorModules;        ///< Generator module library
    std::unique_ptr<OperatorModules> operatorModules;          ///< Operator module library
};

/**
 * @brief Default constructor
 * Initializes PIMPL structure
 */
ShaderManager::ShaderManager() : impl(std::make_unique<Impl>()) {}

/**
 * @brief Destructor
 * Ensures proper cleanup of resources
 */
ShaderManager::~ShaderManager() {
    cleanup();
}

bool ShaderManager::initialize() {
    // Adjust LYGIA path based on executable location
    std::string lygiaPath = "../external/lygia";  // When running from build/ directory
    
    // Check if path exists
    struct stat info;
    if (stat(lygiaPath.c_str(), &info) != 0) {
        // Try current directory
        lygiaPath = "external/lygia";
        if (stat(lygiaPath.c_str(), &info) != 0) {
            std::cerr << "LYGIA path not found: " << lygiaPath << std::endl;
            return false;
        }
    }
    
    return initialize(lygiaPath, "shaders");
}

bool ShaderManager::initialize(const std::string& lygiaPath, const std::string& shaderPath) {
    impl->lygiaPath = lygiaPath;
    impl->shaderPath = shaderPath;
    
    // Initialize module systems
    impl->generatorModules = std::make_unique<GeneratorModules>();
    impl->operatorModules = std::make_unique<OperatorModules>();
    
    // Create fullscreen quad VAO
    impl->fullscreenQuadVAO = createFullscreenQuadVAO();
    
    // Create default fallback shader (OpenGL 4.1 Core)
    std::string defaultVertexSource = R"glsl(
#version 410 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)glsl";

    std::string defaultFragmentSource = R"glsl(
#version 410 core

in vec2 TexCoord;
out vec4 FragColor;

uniform float iTime;
uniform vec2 iResolution;

void main() {
    vec2 uv = TexCoord;
    
    // Simple animated gradient pattern as fallback
    vec3 color = vec3(0.5 + 0.5 * cos(iTime + uv.xyx + vec3(0, 2, 4)));
    FragColor = vec4(color, 1.0);
}
)glsl";

    auto result = createShaderFromSource(defaultVertexSource, defaultFragmentSource);
    if (!result.success) {
        std::cerr << "Failed to create default shader: " << result.errorLog << std::endl;
        return false;
    }
    
    impl->defaultShader = result.program;
    impl->currentShader = impl->defaultShader;
    
    // Discover available LYGIA modules
    if (std::filesystem::exists(impl->lygiaPath)) {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(impl->lygiaPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".glsl") {
                std::string relativePath = std::filesystem::relative(entry.path(), impl->lygiaPath).string();
                impl->availableModules.push_back(relativePath);
            }
        }
        std::cout << "Discovered " << impl->availableModules.size() << " LYGIA modules" << std::endl;
    }
    
    std::cout << "ShaderManager initialized successfully" << std::endl;
    return true;
}

void ShaderManager::cleanup() {
    // Delete all cached shaders
    for (auto& [key, program] : impl->shaderCache) {
        glDeleteProgram(program);
    }
    impl->shaderCache.clear();
    
    // Delete node render targets
    for (auto& [nodeId, target] : impl->nodeRenderTargets) {
        if (target.framebuffer) glDeleteFramebuffers(1, &target.framebuffer);
        if (target.texture) glDeleteTextures(1, &target.texture);
    }
    impl->nodeRenderTargets.clear();
    
    // Delete fullscreen quad VAO
    if (impl->fullscreenQuadVAO) {
        glDeleteVertexArrays(1, &impl->fullscreenQuadVAO);
        impl->fullscreenQuadVAO = 0;
    }
    
    if (impl->defaultShader != 0) {
        glDeleteProgram(impl->defaultShader);
        impl->defaultShader = 0;
    }
    
    impl->moduleCache.clear();
    impl->programSources.clear();
    impl->fileTimestamps.clear();
}

// ====================================================
// ============================================================================
// Core functionality: Pipeline graph-based shader generation
// ============================================================================
// ====================================================

ShaderManager::CompilationResult ShaderManager::generateShaderFromGraph(const PipelineGraph& graph) {
    try {
        // Generate unique cache key from graph topology
        std::string cacheKey = generateGraphCacheKey(graph);
        
        // Check cache first
        auto it = impl->shaderCache.find(cacheKey);
        if (it != impl->shaderCache.end()) {
            impl->cacheHits++;
            return {true, it->second, "", {}};
        }
        
        // Generate vertex shader (standard fullscreen quad)
        std::string vertexShader = R"glsl(
#version 410 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)glsl";

        // Generate fragment shader from graph
        std::string fragmentShader = generateFragmentShaderFromGraph(graph);
        
        // Compile and link
        auto result = createShaderFromSource(vertexShader, fragmentShader);
        
        if (result.success) {
            impl->shaderCache[cacheKey] = result.program;
            impl->programSources[result.program] = fragmentShader;
            
            // Track file dependencies for hot-reload
            if (impl->hotReloadEnabled) {
                trackGraphDependencies(graph, result.program);
            }
        }
        
        impl->compilationCount++;
        return result;
        
    } catch (const std::exception& e) {
        return {false, 0, std::string("Graph compilation error: ") + e.what(), {}};
    }
}

ShaderManager::CompilationResult ShaderManager::generateNodePreviewShader(const PipelineGraph& graph, 
                                                                          int nodeId, 
                                                                          const std::string& outputPort) {
    try {
        // Create a subgraph that includes only dependencies of the target node
        auto subGraph = graph.extractSubgraphTo(nodeId);
        
        // Generate cache key for this specific node preview
        std::string cacheKey = "preview_" + std::to_string(nodeId) + "_" + outputPort + "_" + generateGraphCacheKey(*subGraph);
        
        // Check cache
        auto it = impl->shaderCache.find(cacheKey);
        if (it != impl->shaderCache.end()) {
            impl->cacheHits++;
            return {true, it->second, "", {}};
        }
        
        // Generate vertex shader
        std::string vertexShader = R"glsl(
#version 410 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)glsl";

        // Generate fragment shader focused on the specific node output
        std::string fragmentShader = generateNodePreviewFragmentShader(*subGraph, nodeId, outputPort);
        
        auto result = createShaderFromSource(vertexShader, fragmentShader);
        
        if (result.success) {
            impl->shaderCache[cacheKey] = result.program;
            impl->programSources[result.program] = fragmentShader;
        }
        
        impl->compilationCount++;
        return result;
        
    } catch (const std::exception& e) {
        return {false, 0, std::string("Node preview compilation error: ") + e.what(), {}};
    }
}

// ====================================================
// ============================================================================
// Basic shader management (compatibility)
// ============================================================================
// ====================================================

ShaderManager::CompilationResult ShaderManager::createShaderFromSource(const std::string& vertexSource, 
                                                                       const std::string& fragmentSource) {
    CompilationResult result;
    
    // Process includes in both shaders
    std::string processedVertexSource = processIncludes(vertexSource);
    std::string processedFragmentSource = processIncludes(fragmentSource);
    
    // Compile vertex shader
    GLuint vertexShader = compileShader(processedVertexSource, GL_VERTEX_SHADER);
    if (vertexShader == 0) {
        result.errorLog = "Failed to compile vertex shader";
        return result;
    }
    
    // Compile fragment shader
    GLuint fragmentShader = compileShader(processedFragmentSource, GL_FRAGMENT_SHADER);
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        result.errorLog = "Failed to compile fragment shader";
        return result;
    }
    
    // Link program
    GLuint program = linkProgram(vertexShader, fragmentShader);
    
    // Clean up individual shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    if (program == 0) {
        result.errorLog = "Failed to link shader program";
        return result;
    }
    
    result.success = true;
    result.program = program;
    
    return result;
}

void ShaderManager::deleteShader(GLuint program) {
    if (program == 0) return;
    
    // Remove from caches
    for (auto it = impl->shaderCache.begin(); it != impl->shaderCache.end(); ++it) {
        if (it->second == program) {
            impl->shaderCache.erase(it);
            break;
        }
    }
    
    impl->programSources.erase(program);
    
    // Don't delete the default shader
    if (program != impl->defaultShader) {
        glDeleteProgram(program);
    }
}

// ====================================================
// ============================================================================
// FBO rendering system (for Node Editor)
// ============================================================================
// ====================================================

ShaderManager::RenderTarget ShaderManager::createNodeRenderTarget(int nodeId, int width, int height) {
    // Delete existing target if it exists
    deleteNodeRenderTarget(nodeId);
    
    RenderTarget target;
    target.width = width;
    target.height = height;
    
    // Generate FBO
    glGenFramebuffers(1, &target.framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, target.framebuffer);
    
    // Generate texture
    glGenTextures(1, &target.texture);
    glBindTexture(GL_TEXTURE_2D, target.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // Attach texture to FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.texture, 0);
    
    // Check FBO completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "FBO not complete for node " << nodeId << std::endl;
        glDeleteFramebuffers(1, &target.framebuffer);
        glDeleteTextures(1, &target.texture);
        target.framebuffer = 0;
        target.texture = 0;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    impl->nodeRenderTargets[nodeId] = target;
    return target;
}

bool ShaderManager::renderNodeToFBO(const PipelineGraph& graph, int nodeId, const RenderTarget& renderTarget) {
    if (renderTarget.framebuffer == 0) {
        std::cerr << "Invalid render target for node " << nodeId << std::endl;
        return false;
    }
    
    // Generate preview shader for this node
    auto shaderResult = generateNodePreviewShader(graph, nodeId);
    if (!shaderResult.success) {
        std::cerr << "Failed to generate preview shader for node " << nodeId 
                  << ": " << shaderResult.errorLog << std::endl;
        return false;
    }
    
    // Save current state
    GLint prevFramebuffer, prevViewport[4];
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFramebuffer);
    glGetIntegerv(GL_VIEWPORT, prevViewport);
    GLuint prevProgram = impl->currentShader;
    
    // Bind FBO and set viewport
    glBindFramebuffer(GL_FRAMEBUFFER, renderTarget.framebuffer);
    glViewport(0, 0, renderTarget.width, renderTarget.height);
    
    // Clear
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Use preview shader
    useShader(shaderResult.program);
    
    // Set common uniforms
    setUniform("iTime", static_cast<float>(std::chrono::duration<double>(
        std::chrono::steady_clock::now().time_since_epoch()).count()));
    setUniform("iResolution", static_cast<float>(renderTarget.width), static_cast<float>(renderTarget.height));
    
    // Set node-specific uniforms from graph
    setNodeUniforms(graph, nodeId);
    
    // Render fullscreen quad
    glBindVertexArray(impl->fullscreenQuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    
    // Restore state
    glBindFramebuffer(GL_FRAMEBUFFER, prevFramebuffer);
    glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
    useShader(prevProgram);
    
    return true;
}

void ShaderManager::deleteNodeRenderTarget(int nodeId) {
    auto it = impl->nodeRenderTargets.find(nodeId);
    if (it != impl->nodeRenderTargets.end()) {
        if (it->second.framebuffer) glDeleteFramebuffers(1, &it->second.framebuffer);
        if (it->second.texture) glDeleteTextures(1, &it->second.texture);
        impl->nodeRenderTargets.erase(it);
    }
}

// ====================================================
// ============================================================================
// Shader usage and state
// ============================================================================
// ====================================================

void ShaderManager::useShader(GLuint program) {
    if (program != impl->currentShader) {
        glUseProgram(program);
        impl->currentShader = program;
    }
}

GLuint ShaderManager::getCurrentShader() const {
    return impl->currentShader;
}

GLuint ShaderManager::getDefaultShader() const {
    return impl->defaultShader;
}

// ====================================================
// ============================================================================
// Uniform management
// ============================================================================
// ====================================================

void ShaderManager::setUniform(const std::string& name, float value) {
    if (impl->currentShader == 0) return;
    
    GLint location = glGetUniformLocation(impl->currentShader, name.c_str());
    if (location != -1) {
        glUniform1f(location, value);
    }
}

void ShaderManager::setUniform(const std::string& name, float x, float y) {
    if (impl->currentShader == 0) return;
    
    GLint location = glGetUniformLocation(impl->currentShader, name.c_str());
    if (location != -1) {
        glUniform2f(location, x, y);
    }
}

void ShaderManager::setUniform(const std::string& name, float x, float y, float z) {
    if (impl->currentShader == 0) return;
    
    GLint location = glGetUniformLocation(impl->currentShader, name.c_str());
    if (location != -1) {
        glUniform3f(location, x, y, z);
    }
}

void ShaderManager::setUniform(const std::string& name, float x, float y, float z, float w) {
    if (impl->currentShader == 0) return;
    
    GLint location = glGetUniformLocation(impl->currentShader, name.c_str());
    if (location != -1) {
        glUniform4f(location, x, y, z, w);
    }
}

// ====================================================
// ============================================================================
// LYGIA module management
// ============================================================================
// ====================================================

std::string ShaderManager::loadLygiaModule(const std::string& moduleName) {
    // Check cache first
    auto it = impl->moduleCache.find(moduleName);
    if (it != impl->moduleCache.end()) {
        return it->second;
    }
    
    // Load from file
    std::string fullPath = impl->lygiaPath + "/" + moduleName;
    std::string content = loadFile(fullPath);
    
    if (!content.empty()) {
        impl->moduleCache[moduleName] = content;
    }
    
    return content;
}

std::vector<std::string> ShaderManager::getAvailableModules() const {
    return impl->availableModules;
}

// ====================================================
// ============================================================================
// Hot-reload and file watching
// ============================================================================
// ====================================================

void ShaderManager::enableHotReload(bool enable) {
    impl->hotReloadEnabled = enable;
    
    if (enable) {
        std::cout << "Hot-reload enabled" << std::endl;
    } else {
        impl->fileTimestamps.clear();
        std::cout << "Hot-reload disabled" << std::endl;
    }
}

void ShaderManager::checkForChanges() {
    if (!impl->hotReloadEnabled) return;
    
    // Check LYGIA module files
    for (const auto& module : impl->availableModules) {
        std::string fullPath = impl->lygiaPath + "/" + module;
        
        if (std::filesystem::exists(fullPath)) {
            auto currentTime = std::filesystem::last_write_time(fullPath);
            auto it = impl->fileTimestamps.find(fullPath);
            
            if (it != impl->fileTimestamps.end() && it->second != currentTime) {
                // File was modified
                std::cout << "Detected change in: " << module << std::endl;
                
                // Clear module cache
                impl->moduleCache.erase(module);
                
                // Trigger reload callback
                if (impl->reloadCallback) {
                    impl->reloadCallback(0, fullPath);  // 0 indicates module change
                }
                
                impl->hotReloads++;
            }
            
            impl->fileTimestamps[fullPath] = currentTime;
        }
    }
}

void ShaderManager::setReloadCallback(std::function<void(GLuint, const std::string&)> callback) {
    impl->reloadCallback = callback;
}

// ====================================================
// ============================================================================
// Statistics and monitoring
// ============================================================================
// ====================================================

std::unordered_map<std::string, int> ShaderManager::getStatistics() const {
    return {
        {"compilations", impl->compilationCount},
        {"cache_hits", impl->cacheHits},
        {"hot_reloads", impl->hotReloads},
        {"cached_shaders", static_cast<int>(impl->shaderCache.size())},
        {"cached_modules", static_cast<int>(impl->moduleCache.size())},
        {"available_modules", static_cast<int>(impl->availableModules.size())},
        {"render_targets", static_cast<int>(impl->nodeRenderTargets.size())}
    };
}

int ShaderManager::getCacheHits() const {
    return impl->cacheHits;
}

int ShaderManager::getHotReloads() const {
    return impl->hotReloads;
}

int ShaderManager::getCompilationCount() const {
    return impl->compilationCount;
}

void ShaderManager::clearCaches() {
    // Don't delete current or default shader
    for (auto& [key, program] : impl->shaderCache) {
        if (program != impl->currentShader && program != impl->defaultShader) {
            glDeleteProgram(program);
        }
    }
    
    impl->shaderCache.clear();
    impl->moduleCache.clear();
    impl->programSources.clear();
    
    // Re-add default shader to cache
    if (impl->defaultShader != 0) {
        impl->shaderCache["__default__"] = impl->defaultShader;
    }
    
    std::cout << "All caches cleared" << std::endl;
}

// ====================================================
// ============================================================================
// Private helper functions
// ============================================================================
// ====================================================

GLuint ShaderManager::compileShader(const std::string& source, GLenum shaderType) {
    GLuint shader = glCreateShader(shaderType);
    const char* sourceCStr = source.c_str();
    glShaderSource(shader, 1, &sourceCStr, nullptr);
    glCompileShader(shader);
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    
    if (!success) {
        GLchar infoLog[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
        std::cerr << "Shader compilation error (" 
                  << (shaderType == GL_VERTEX_SHADER ? "vertex" : "fragment") 
                  << "):\n" << infoLog << std::endl;
        std::cerr << "Source:\n" << source << std::endl;
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
    
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    
    if (!success) {
        GLchar infoLog[1024];
        glGetProgramInfoLog(program, 1024, nullptr, infoLog);
        std::cerr << "Program linking error:\n" << infoLog << std::endl;
        glDeleteProgram(program);
        return 0;
    }
    
    return program;
}

std::string ShaderManager::loadFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << path << std::endl;
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string ShaderManager::processIncludes(const std::string& source) {
    std::string result = source;
    std::regex includeRegex(R"(#include\s*[<"]([^>"]+)[>"])");
    std::smatch match;
    
    while (std::regex_search(result, match, includeRegex)) {
        std::string includePath = match[1].str();
        std::string includeContent;
        
        // Try to load from LYGIA first
        if (includePath.find("lygia/") == 0) {
            std::string lygiaModule = includePath.substr(6); // Remove "lygia/" prefix
            includeContent = loadLygiaModule(lygiaModule);
        } else if (includePath.find("../") == 0) {
            // Handle LYGIA relative paths like "../math/mod289.glsl"
            std::string lygiaModule = includePath.substr(3); // Remove "../" prefix
            includeContent = loadLygiaModule(lygiaModule);
        } else {
            // Try relative to shader path
            std::string fullPath = impl->shaderPath + "/" + includePath;
            includeContent = loadFile(fullPath);
        }
        
        if (includeContent.empty()) {
            std::cerr << "Warning: Could not load include: " << includePath << std::endl;
            includeContent = "// Include not found: " + includePath;
        }
        
        // Recursively process includes in the included content
        includeContent = processIncludes(includeContent);
        
        result.replace(match.position(), match.length(), includeContent);
    }
    
    return result;
}

GLuint ShaderManager::createFullscreenQuadVAO() {
    // Fullscreen quad vertices
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    
    // position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
    
    return VAO;
}

// ====================================================
// ============================================================================
// Graph-related helper functions (implementation needed)
// ============================================================================
// ====================================================

std::string ShaderManager::generateGraphCacheKey(const PipelineGraph& graph) {
    // TODO: Generate unique hash from graph topology and node parameters
    std::stringstream ss;
    
    // Include all nodes and their connections in the key
    for (const auto& node : graph.getNodes()) {
        ss << node->id << ":" << (node->module ? node->module->getName() : "unknown") << ";";
        for (const auto& [key, value] : node->parameters) {
            ss << key << "=" << value << ";";
        }
    }
    
    for (const auto& connection : graph.getConnections()) {
        ss << connection.fromNodeId << "->" << connection.toNodeId << ":" << connection.fromPort << "->" << connection.toPort << ";";
    }
    
    return ss.str();
}

std::string ShaderManager::generateFragmentShaderFromGraph(const PipelineGraph& graph) {
    std::stringstream shader;
    
    // Shader header
    shader << "#version 410 core\n\n";
    shader << "in vec2 TexCoord;\n";
    shader << "out vec4 FragColor;\n\n";
    
    // Common uniforms
    shader << "uniform float iTime;\n";
    shader << "uniform vec2 iResolution;\n\n";
    
    // Include required LYGIA modules
    std::set<std::string> requiredModules = graph.getRequiredLygiaModules();
    for (const auto& module : requiredModules) {
        std::string moduleContent = loadLygiaModule(module);
        if (!moduleContent.empty()) {
            shader << "// Module: " << module << "\n";
            shader << moduleContent << "\n\n";
        }
    }
    
    // Generate node functions
    for (const auto& node : graph.getTopologicalOrder()) {
        shader << generateNodeFunction(node) << "\n\n";
    }
    
    // Main function
    shader << "void main() {\n";
    shader << "    vec2 uv = TexCoord;\n";
    shader << "    \n";
    
    // Declare node output variables
    for (const auto& node : graph.getTopologicalOrder()) {
        shader << "    vec4 " << node.getName() << "_output;\n";
    }
    shader << "    \n";
    
    // Execute nodes in topological order
    for (const auto& node : graph.getTopologicalOrder()) {
        shader << "    " << generateNodeCall(node) << "\n";
    }
    
    // Output final result
    auto outputNodes = graph.getOutputNodes();
    if (!outputNodes.empty()) {
        shader << "    \n";
        shader << "    FragColor = " << outputNodes[0].getName() << "_output;\n";
    } else {
        shader << "    \n";
        shader << "    FragColor = vec4(uv, 0.5, 1.0); // No output nodes\n";
    }
    
    shader << "}\n";
    
    return shader.str();
}

std::string ShaderManager::generateNodePreviewFragmentShader(const PipelineGraph& graph, int nodeId, const std::string& outputPort) {
    // Similar to generateFragmentShaderFromGraph but focuses on specific node output
    std::stringstream shader;
    
    shader << "#version 410 core\n\n";
    shader << "in vec2 TexCoord;\n";
    shader << "out vec4 FragColor;\n\n";
    
    shader << "uniform float iTime;\n";
    shader << "uniform vec2 iResolution;\n\n";
    
    // Include required modules for subgraph
    std::set<std::string> requiredModules = graph.getRequiredLygiaModules();
    for (const auto& module : requiredModules) {
        std::string moduleContent = loadLygiaModule(module);
        if (!moduleContent.empty()) {
            shader << "// Module: " << module << "\n";
            shader << moduleContent << "\n\n";
        }
    }
    
    // Generate functions for dependencies only
    auto dependencies = graph.getDependenciesOf(nodeId);
    for (const auto& node : dependencies) {
        shader << generateNodeFunction(node) << "\n\n";
    }
    
    shader << "void main() {\n";
    shader << "    vec2 uv = TexCoord;\n";
    shader << "    \n";
    
    // Declare node output variables
    for (const auto& node : dependencies) {
        shader << "    vec4 " << node.getName() << "_output;\n";
    }
    shader << "    \n";
    
    // Execute dependencies
    for (const auto& node : dependencies) {
        shader << "    " << generateNodeCall(node) << "\n";
    }
    
    // Output the specific node's result
    auto targetNode = graph.getNode(nodeId);
    shader << "    \n";
    shader << "    FragColor = " << targetNode.getName() << "_" << outputPort << ";\n";
    shader << "}\n";
    
    return shader.str();
}

std::string ShaderManager::generateNodeFunction(const PipelineGraph::Node& node) {
    // Delegate to module systems based on node type
    if (impl->generatorModules && impl->generatorModules->hasGenerator(node.getType())) {
        return impl->generatorModules->generateFunction(node);
    } else if (impl->operatorModules && impl->operatorModules->hasOperator(node.getType())) {
        return impl->operatorModules->generateFunction(node);
    } else {
        // Fallback for unknown node types
        return "// Unknown node type: " + node.getType();
    }
}

std::string ShaderManager::generateNodeCall(const PipelineGraph::Node& node) {
    // Generate function call for the node
    return node.getName() + "_output = " + node.getName() + "_func(" + generateNodeInputs(node) + ");";
}

std::string ShaderManager::generateNodeInputs(const PipelineGraph::Node& node) {
    // Generate comma-separated list of inputs for the node
    std::vector<std::string> inputs;
    
    // Add common inputs
    inputs.push_back("uv");
    inputs.push_back("iTime");
    
    // Add node-specific parameters
    for (const auto& [key, value] : node.getParameters()) {
        inputs.push_back(value);
    }
    
    // Add connected inputs
    // TODO: Get connections from graph and add connected outputs
    
    std::stringstream result;
    for (size_t i = 0; i < inputs.size(); ++i) {
        if (i > 0) result << ", ";
        result << inputs[i];
    }
    
    return result.str();
}

void ShaderManager::trackGraphDependencies(const PipelineGraph& graph, GLuint program) {
    // Track all LYGIA modules used by this graph for hot-reload
    auto modules = graph.getRequiredLygiaModules();
    for (const auto& module : modules) {
        std::string fullPath = impl->lygiaPath + "/" + module;
        if (std::filesystem::exists(fullPath)) {
            impl->fileTimestamps[fullPath] = std::filesystem::last_write_time(fullPath);
        }
    }
}

void ShaderManager::setNodeUniforms(const PipelineGraph& graph, int nodeId) {
    // Set uniforms specific to the node from the graph
    auto node = graph.getNode(nodeId);
    
    for (const auto& [key, value] : node.getParameters()) {
        // Try to parse as different types and set uniforms
        try {
            float floatValue = std::stof(value);
            setUniform("node_" + std::to_string(nodeId) + "_" + key, floatValue);
        } catch (...) {
            // Could be vec2, vec3, vec4, or other types
            // TODO: Add more sophisticated parameter parsing
        }
    }
}
