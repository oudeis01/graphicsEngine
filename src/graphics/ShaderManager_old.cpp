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
    return initialize("external/lygia", "shaders");
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
    discoverLygiaModules();
    
    std::cout << "ShaderManager initialized with " << impl->availableModules.size() 
              << " LYGIA modules" << std::endl;
    
    return true;
}
    vec3 color = vec3(
        0.5 + 0.5 * sin(iTime + uv.x * 6.28318),
        0.5 + 0.5 * sin(iTime + uv.y * 6.28318 + 2.094),
        0.5 + 0.5 * sin(iTime + (uv.x + uv.y) * 6.28318 + 4.188)
    );
    
    FragColor = vec4(color, 1.0);
}
)glsl";

    auto result = createShaderFromSource(defaultVertexSource, defaultFragmentSource);
    if (!result.success) {
        std::cerr << "Failed to create default shader: " << result.errorLog << std::endl;
        return false;
    }
    
    impl->defaultShader = result.program;
    
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
    
    if (impl->defaultShader != 0) {
        glDeleteProgram(impl->defaultShader);
        impl->defaultShader = 0;
    }
    
    impl->moduleCache.clear();
    impl->programSources.clear();
    impl->fileTimestamps.clear();
}

ShaderManager::CompilationResult ShaderManager::loadShader(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string cacheKey = vertexPath + "|" + fragmentPath;
    
    // Check cache first
    auto it = impl->shaderCache.find(cacheKey);
    if (it != impl->shaderCache.end()) {
        impl->cacheHits++;
        return {true, it->second, "", {}};
    }

    std::string vertexSource = loadFile(vertexPath);
    std::string fragmentSource = loadFile(fragmentPath);
    
    if (vertexSource.empty() || fragmentSource.empty()) {
        return {false, 0, "Failed to load shader files: " + vertexPath + ", " + fragmentPath, {}};
    }

    auto result = createShaderFromSource(vertexSource, fragmentSource);
    if (result.success) {
        impl->shaderCache[cacheKey] = result.program;
        
        // Track file timestamps for hot-reload
        if (impl->hotReloadEnabled) {
            if (std::filesystem::exists(vertexPath)) {
                impl->fileTimestamps[vertexPath] = std::filesystem::last_write_time(vertexPath);
            }
            if (std::filesystem::exists(fragmentPath)) {
                impl->fileTimestamps[fragmentPath] = std::filesystem::last_write_time(fragmentPath);
            }
        }
    }
    
    return result;
}

ShaderManager::CompilationResult ShaderManager::createShaderFromSource(const std::string& vertexSource, const std::string& fragmentSource) {
    CompilationResult result;
    
    // Process includes
    std::string processedVertexSource = processIncludes(vertexSource);
    std::string processedFragmentSource = processIncludes(fragmentSource);

    GLuint vertexShader = compileShader(processedVertexSource, GL_VERTEX_SHADER);
    if (vertexShader == 0) {
        result.errorLog = "Vertex shader compilation failed";
        return result;
    }

    GLuint fragmentShader = compileShader(processedFragmentSource, GL_FRAGMENT_SHADER);
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        result.errorLog = "Fragment shader compilation failed";
        return result;
    }

    GLuint program = linkProgram(vertexShader, fragmentShader);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    if (program == 0) {
        result.errorLog = "Shader program linking failed";
        return result;
    }
    
    result.success = true;
    result.program = program;
    impl->compilationCount++;
    
    // Store source for debugging
    impl->programSources[program] = "VERTEX:\n" + processedVertexSource + "\n\nFRAGMENT:\n" + processedFragmentSource;
    
    return result;
}

void ShaderManager::deleteShader(GLuint program) {
    if (program == 0) return;
    
    glDeleteProgram(program);
    
    // Remove from caches
    for (auto it = impl->shaderCache.begin(); it != impl->shaderCache.end(); ++it) {
        if (it->second == program) {
            impl->shaderCache.erase(it);
            break;
        }
    }
    
    impl->programSources.erase(program);
}

ShaderManager::CompilationResult ShaderManager::generateUberShader(const UberShaderOptions& options) {
    std::string vertexSource = generateUberVertexShader(options);
    std::string fragmentSource = generateUberFragmentShader(options);
    
    return createShaderFromSource(vertexSource, fragmentSource);
}

GLuint ShaderManager::generateUberShader(const std::vector<std::string>& modules) {
    UberShaderOptions options;
    options.modules = modules;
    auto result = generateUberShader(options);
    return result.success ? result.program : 0;
}

void ShaderManager::useShader(GLuint program) {
    if (program == 0) {
        program = impl->defaultShader;
    }
    
    impl->currentShader = program;
    glUseProgram(program);
}

GLuint ShaderManager::getCurrentShader() const {
    return impl->currentShader;
}

GLuint ShaderManager::getDefaultShader() const {
    return impl->defaultShader;
}

void ShaderManager::setUniform(const std::string& name, float value) {
    if (impl->currentShader != 0) {
        GLint location = glGetUniformLocation(impl->currentShader, name.c_str());
        if (location != -1) {
            glUniform1f(location, value);
        }
    }
}

void ShaderManager::setUniform(const std::string& name, float x, float y) {
    if (impl->currentShader != 0) {
        GLint location = glGetUniformLocation(impl->currentShader, name.c_str());
        if (location != -1) {
            glUniform2f(location, x, y);
        }
    }
}

void ShaderManager::setUniform(const std::string& name, float x, float y, float z) {
    if (impl->currentShader != 0) {
        GLint location = glGetUniformLocation(impl->currentShader, name.c_str());
        if (location != -1) {
            glUniform3f(location, x, y, z);
        }
    }
}

void ShaderManager::setUniform(const std::string& name, float x, float y, float z, float w) {
    if (impl->currentShader != 0) {
        GLint location = glGetUniformLocation(impl->currentShader, name.c_str());
        if (location != -1) {
            glUniform4f(location, x, y, z, w);
        }
    }
}

void ShaderManager::setUniform(const std::string& name, int value) {
    if (impl->currentShader != 0) {
        GLint location = glGetUniformLocation(impl->currentShader, name.c_str());
        if (location != -1) {
            glUniform1i(location, value);
        }
    }
}

void ShaderManager::setUniform(const std::string& name, bool value) {
    setUniform(name, value ? 1 : 0);
}

void ShaderManager::setUniformMatrix4(const std::string& name, const float* matrix) {
    if (impl->currentShader != 0) {
        GLint location = glGetUniformLocation(impl->currentShader, name.c_str());
        if (location != -1) {
            glUniformMatrix4fv(location, 1, GL_FALSE, matrix);
        }
    }
}

std::string ShaderManager::loadLygiaModule(const std::string& moduleName) {
    auto it = impl->moduleCache.find(moduleName);
    if (it != impl->moduleCache.end()) {
        return it->second.source;
    }

    std::string modulePath = impl->lygiaPath + "/" + moduleName;
    std::string moduleCode = loadFile(modulePath);
    
    if (!moduleCode.empty()) {
        ShaderModule module;
        module.name = moduleName;
        module.source = moduleCode;
        if (std::filesystem::exists(modulePath)) {
            module.lastModified = std::filesystem::last_write_time(modulePath);
        }
        impl->moduleCache[moduleName] = module;
    }
    
    return moduleCode;
}

void ShaderManager::registerModule(const ShaderModule& module) {
    impl->moduleCache[module.name] = module;
}

std::vector<std::string> ShaderManager::getAvailableModules() const {
    return impl->availableModules;
}

void ShaderManager::enableHotReload(bool enable) {
    impl->hotReloadEnabled = enable;
    if (enable) {
        std::cout << "Hot-reload enabled for ShaderManager" << std::endl;
    }
}

void ShaderManager::checkForChanges() {
    if (!impl->hotReloadEnabled) return;
    
    for (auto& [filepath, lastTime] : impl->fileTimestamps) {
        if (std::filesystem::exists(filepath)) {
            auto currentTime = std::filesystem::last_write_time(filepath);
            if (currentTime > lastTime) {
                std::cout << "Detected change in: " << filepath << std::endl;
                impl->fileTimestamps[filepath] = currentTime;
                impl->hotReloads++;
                
                // Trigger reload callback
                if (impl->reloadCallback) {
                    impl->reloadCallback(0, filepath); // 0 as placeholder for program ID
                }
            }
        }
    }
}

void ShaderManager::setReloadCallback(std::function<void(GLuint, const std::string&)> callback) {
    impl->reloadCallback = callback;
}

std::vector<std::string> ShaderManager::getUniformNames(GLuint program) const {
    std::vector<std::string> uniforms;
    
    if (program == 0) return uniforms;
    
    GLint uniformCount;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniformCount);
    
    for (GLint i = 0; i < uniformCount; ++i) {
        char name[256];
        GLsizei length;
        GLint size;
        GLenum type;
        glGetActiveUniform(program, i, sizeof(name), &length, &size, &type, name);
        uniforms.emplace_back(name);
    }
    
    return uniforms;
}

std::string ShaderManager::getShaderLog(GLuint program) const {
    auto it = impl->programSources.find(program);
    return it != impl->programSources.end() ? it->second : "Program source not available";
}

std::unordered_map<std::string, int> ShaderManager::getStatistics() const {
    return {
        {"compilations", impl->compilationCount},
        {"cache_hits", impl->cacheHits},
        {"hot_reloads", impl->hotReloads},
        {"cached_shaders", static_cast<int>(impl->shaderCache.size())},
        {"cached_modules", static_cast<int>(impl->moduleCache.size())},
        {"available_modules", static_cast<int>(impl->availableModules.size())}
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
    // Delete all cached shaders
    for (auto& [key, program] : impl->shaderCache) {
        glDeleteProgram(program);
    }
    impl->shaderCache.clear();
    impl->moduleCache.clear();
    impl->programSources.clear();
    impl->fileTimestamps.clear();
    
    std::cout << "All shader caches cleared" << std::endl;
}

GLuint ShaderManager::compileShader(const std::string& source, GLenum shaderType) {
    GLuint shader = glCreateShader(shaderType);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
        std::cerr << "Shader compilation failed (" 
                  << (shaderType == GL_VERTEX_SHADER ? "vertex" : "fragment") 
                  << "): " << infoLog << std::endl;
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
        char infoLog[1024];
        glGetProgramInfoLog(program, 1024, nullptr, infoLog);
        std::cerr << "Program linking failed: " << infoLog << std::endl;
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
    std::regex includeRegex(R"(#include\s*[<"]([^>"]*)[>"])");
    std::smatch match;
    
    // Prevent infinite recursion
    int maxDepth = 10;
    int depth = 0;
    
    while (std::regex_search(result, match, includeRegex) && depth < maxDepth) {
        std::string includePath = match[1].str();
        std::string includeContent;
        
        // Try shader path first, then LYGIA
        std::string fullPath = impl->shaderPath + "/" + includePath;
        includeContent = loadFile(fullPath);
        
        if (includeContent.empty()) {
            includeContent = loadLygiaModule(includePath);
        }
        
        if (!includeContent.empty()) {
            result.replace(match.position(), match.length(), includeContent);
        } else {
            std::cerr << "Warning: Could not find include file: " << includePath << std::endl;
            result.replace(match.position(), match.length(), "// Include not found: " + includePath);
        }
        
        depth++;
    }
    
    return result;
}

std::string ShaderManager::generateUberVertexShader(const UberShaderOptions& options) {
    std::stringstream vs;
    
    // Header
    vs << "#version 410 core\n\n";
    
    // Add defines
    for (const auto& [name, value] : options.defines) {
        vs << "#define " << name << " " << value << "\n";
    }
    vs << "\n";
    
    // Standard attributes
    vs << "layout (location = 0) in vec2 aPos;\n";
    vs << "layout (location = 1) in vec2 aTexCoord;\n\n";
    
    // Standard outputs
    vs << "out vec2 TexCoord;\n";
    vs << "out vec2 FragPos;\n\n";
    
    // Include required modules
    for (const auto& moduleName : options.modules) {
        std::string moduleCode = loadLygiaModule(moduleName);
        if (!moduleCode.empty()) {
            vs << "// Module: " << moduleName << "\n";
            vs << moduleCode << "\n\n";
        }
    }
    
    // Main function
    vs << "void main() {\n";
    vs << "    gl_Position = vec4(aPos, 0.0, 1.0);\n";
    vs << "    TexCoord = aTexCoord;\n";
    vs << "    FragPos = aPos;\n";
    vs << "}\n";
    
    return vs.str();
}

std::string ShaderManager::generateUberFragmentShader(const UberShaderOptions& options) {
    std::stringstream fs;
    
    // Header
    fs << "#version 410 core\n\n";
    
    // Add defines
    for (const auto& [name, value] : options.defines) {
        fs << "#define " << name << " " << value << "\n";
    }
    fs << "\n";
    
    // Standard inputs
    fs << "in vec2 TexCoord;\n";
    fs << "in vec2 FragPos;\n\n";
    
    // Standard uniforms
    fs << "uniform float iTime;\n";
    fs << "uniform vec2 iResolution;\n";
    fs << "uniform float iTimeDelta;\n";
    fs << "uniform int iFrame;\n\n";
    
    // Output
    fs << "out vec4 FragColor;\n\n";
    
    // Include required modules
    for (const auto& moduleName : options.modules) {
        std::string moduleCode = loadLygiaModule(moduleName);
        if (!moduleCode.empty()) {
            fs << "// Module: " << moduleName << "\n";
            fs << moduleCode << "\n\n";
        }
    }
    
    // Main function (basic implementation)
    fs << "void main() {\n";
    fs << "    vec2 uv = TexCoord;\n";
    fs << "    vec3 color = vec3(0.0);\n\n";
    
    // Add module-specific code generation here
    // This would be expanded based on the pipeline description
    if (std::find(options.modules.begin(), options.modules.end(), "generative/noise.glsl") != options.modules.end()) {
        fs << "    // Noise generation\n";
        fs << "    color += vec3(noise(uv * 10.0 + iTime));\n\n";
    }
    
    if (std::find(options.modules.begin(), options.modules.end(), "generative/voronoi.glsl") != options.modules.end()) {
        fs << "    // Voronoi pattern\n";
        fs << "    color += vec3(voronoi(uv * 5.0).x);\n\n";
    }
    
    // Default fallback
    if (options.modules.empty()) {
        fs << "    // Default gradient\n";
        fs << "    color = vec3(uv, 0.5 + 0.5 * sin(iTime));\n";
    }
    
    fs << "    FragColor = vec4(color, 1.0);\n";
    fs << "}\n";
    
    return fs.str();
}
