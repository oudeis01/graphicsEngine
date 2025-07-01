#pragma once

#include <GL/glew.h>
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>
#include <filesystem>
#include <chrono>
#include "PipelineGraph.h"

/**
 * @brief 고급 GLSL 셰이더 관리 및 동적 우버 셰이더 생성 시스템
 * 
 * 책임:
 * - PipelineGraph 기반 동적 셰이더 생성
 * - LYGIA 라이브러리 모듈 조합
 * - 노드별 FBO 렌더링 지원
 * - 실시간 hot-reloading
 * - 포괄적인 유니폼 관리
 */
class ShaderManager {
public:
    /**
     * @brief 셰이더 컴파일 결과 구조체
     */
    struct CompilationResult {
        bool success = false;               ///< 컴파일 성공 플래그
        GLuint program = 0;                 ///< OpenGL 프로그램 ID
        std::string errorLog;               ///< 컴파일 에러 메시지
        std::vector<std::string> warnings;  ///< 경고 메시지
    };

    /**
     * @brief FBO 렌더링 결과 구조체
     */
    struct RenderTarget {
        GLuint framebuffer = 0;             ///< FBO ID
        GLuint texture = 0;                 ///< 결과 텍스처 ID
        int width = 256;                    ///< 텍스처 너비
        int height = 256;                   ///< 텍스처 높이
    };

public:
    ShaderManager();
    ~ShaderManager();

    // 핵심 초기화 및 생명주기
    /**
     * @brief 기본 설정으로 셰이더 매니저 초기화
     * @return 초기화 성공 여부
     */
    bool initialize();
    
    /**
     * @brief 사용자 정의 LYGIA 및 셰이더 경로로 초기화
     * @param lygiaPath LYGIA 라이브러리 디렉토리 경로
     * @param shaderPath 셰이더 파일 디렉토리 경로
     * @return 초기화 성공 여부
     */
    bool initialize(const std::string& lygiaPath, const std::string& shaderPath);
    
    /**
     * @brief 모든 셰이더 리소스 및 캐시 정리
     */
    void cleanup();

    // 파이프라인 그래프 기반 셰이더 생성
    /**
     * @brief 파이프라인 그래프로부터 우버 셰이더 생성
     * @param graph 파이프라인 그래프
     * @return 컴파일 결과 (프로그램 ID 또는 에러 정보)
     */
    CompilationResult generateShaderFromGraph(const PipelineGraph& graph);
    
    /**
     * @brief 특정 노드의 미리보기 셰이더 생성 (노드 에디터용)
     * @param graph 파이프라인 그래프
     * @param nodeId 미리보기할 노드 ID
     * @param outputPort 출력 포트 이름
     * @return 컴파일 결과
     */
    CompilationResult generateNodePreviewShader(const PipelineGraph& graph, 
                                               int nodeId, 
                                               const std::string& outputPort = "output");

    // 기본 셰이더 관리 (호환성)
    /**
     * @brief 소스 문자열로부터 셰이더 프로그램 생성
     * @param vertexSource 버텍스 셰이더 소스 코드
     * @param fragmentSource 프래그먼트 셰이더 소스 코드
     * @return 컴파일 결과
     */
    CompilationResult createShaderFromSource(const std::string& vertexSource, const std::string& fragmentSource);
    
    /**
     * @brief 셰이더 프로그램 삭제 및 캐시에서 제거
     * @param program 삭제할 OpenGL 프로그램 ID
     */
    void deleteShader(GLuint program);

    // FBO 렌더링 시스템 (노드 에디터용)
    /**
     * @brief 노드별 FBO 렌더 타겟 생성
     * @param nodeId 노드 ID
     * @param width FBO 너비
     * @param height FBO 높이
     * @return 생성된 렌더 타겟
     */
    RenderTarget createNodeRenderTarget(int nodeId, int width = 256, int height = 256);
    
    /**
     * @brief 특정 노드를 FBO에 렌더링
     * @param graph 파이프라인 그래프
     * @param nodeId 렌더링할 노드 ID
     * @param renderTarget 대상 렌더 타겟
     * @return 렌더링 성공 여부
     */
    bool renderNodeToFBO(const PipelineGraph& graph, int nodeId, const RenderTarget& renderTarget);
    
    /**
     * @brief 노드 렌더 타겟 삭제
     * @param nodeId 노드 ID
     */
    void deleteNodeRenderTarget(int nodeId);

    // 셰이더 사용 및 상태
    /**
     * @brief 셰이더 프로그램 바인딩
     * @param program 사용할 OpenGL 프로그램 ID
     */
    void useShader(GLuint program);
    
    /**
     * @brief 현재 바인딩된 셰이더 프로그램 반환
     * @return 현재 활성화된 OpenGL 프로그램 ID
     */
    GLuint getCurrentShader() const;
    
    /**
     * @brief 기본 fallback 셰이더 프로그램 반환
     * @return 기본 셰이더 프로그램 ID
     */
    GLuint getDefaultShader() const;

    // 유니폼 관리
    /**
     * @brief float 유니폼 값 설정
     * @param name 유니폼 변수 이름
     * @param value 설정할 float 값
     */
    void setUniform(const std::string& name, float value);
    
    /**
     * @brief vec2 유니폼 값 설정
     * @param name 유니폼 변수 이름
     * @param x X 컴포넌트
     * @param y Y 컴포넌트
     */
    void setUniform(const std::string& name, float x, float y);
        bool enableOptimization = true;     ///< Enable shader optimization
        bool enableDebugging = false;       ///< Include debug information
    };

public:
    ShaderManager();
    ~ShaderManager();

    // Core initialization and lifecycle
    /**
     * @brief Initialize shader manager with default settings
     * @return true if initialization successful, false otherwise
     */
    bool initialize();
    
    /**
     * @brief Initialize with custom LYGIA and shader paths
     * @param lygiaPath Path to LYGIA library directory
     * @param shaderPath Path to shader files directory
     * @return true if initialization successful, false otherwise
     */
    bool initialize(const std::string& lygiaPath, const std::string& shaderPath);
    
    /**
     * @brief Clean up all shader resources and caches
     */
    void cleanup();

    // Basic shader management
    /**
     * @brief Load shader program from vertex and fragment files
     * @param vertexPath Path to vertex shader file
     * @param fragmentPath Path to fragment shader file
     * @return Compilation result with program ID or error information
     */
    CompilationResult loadShader(const std::string& vertexPath, const std::string& fragmentPath);
    
    /**
     * @brief Create shader program from source strings
     * @param vertexSource Vertex shader source code
     * @param fragmentSource Fragment shader source code
     * @return Compilation result with program ID or error information
     */
    CompilationResult createShaderFromSource(const std::string& vertexSource, const std::string& fragmentSource);
    
    /**
     * @brief Delete shader program and remove from cache
     * @param program OpenGL program ID to delete
     */
    void deleteShader(GLuint program);

    // Uber shader generation
    /**
     * @brief Generate uber shader with specified modules
     * @param options Configuration for uber shader generation
     * @return Compilation result with generated uber shader
     */
    CompilationResult generateUberShader(const UberShaderOptions& options);
    
    /**
     * @brief Generate uber shader from module list (simplified interface)
     * @param modules List of module names to include
     * @return OpenGL program ID or 0 on failure
     */
    GLuint generateUberShader(const std::vector<std::string>& modules);

    // Shader usage and state
    /**
     * @brief Bind shader program for rendering
     * @param program OpenGL program ID to use
     */
    void useShader(GLuint program);
    
    /**
     * @brief Get currently bound shader program
     * @return Currently active OpenGL program ID
     */
    GLuint getCurrentShader() const;
    
    /**
     * @brief Get default fallback shader program
     * @return Default shader program ID
     */
    GLuint getDefaultShader() const;

    // Uniform management
    /**
     * @brief Set float uniform value
     * @param name Uniform variable name
     * @param value Float value to set
     */
    void setUniform(const std::string& name, float value);
    
    /**
     * @brief Set vec2 uniform value
     * @param name Uniform variable name
     * @param x X component
     * @param y Y component
     */
    void setUniform(const std::string& name, float x, float y);
    
    /**
     * @brief Set vec3 uniform value
     * @param name Uniform variable name
     * @param x X component
     * @param y Y component
     * @param z Z component
     */
    void setUniform(const std::string& name, float x, float y, float z);
    
    /**
     * @brief Set vec4 uniform value
     * @param name Uniform variable name
     * @param x X component
     * @param y Y component
     * @param z Z component
     * @param w W component
     */
    void setUniform(const std::string& name, float x, float y, float z, float w);
    
    /**
     * @brief Set integer uniform value
     * @param name Uniform variable name
     * @param value Integer value to set
     */
    void setUniform(const std::string& name, int value);
    
    /**
     * @brief Set boolean uniform value
     * @param name Uniform variable name
     * @param value Boolean value to set
     */
    void setUniform(const std::string& name, bool value);
    
    /**
     * @brief Set matrix4 uniform value
     * @param name Uniform variable name
     * @param matrix Pointer to 16 float matrix values
     */
    void setUniformMatrix4(const std::string& name, const float* matrix);

    // LYGIA module system
    /**
     * @brief Load LYGIA module by name
     * @param moduleName Module name (e.g., "math/noise.glsl")
     * @return Module source code or empty string on failure
     */
    std::string loadLygiaModule(const std::string& moduleName);
    
    /**
     * @brief Register custom shader module
     * @param module Module information to register
     */
    void registerModule(const ShaderModule& module);
    
    /**
     * @brief Get all available modules
     * @return Vector of all registered module names
     */
    std::vector<std::string> getAvailableModules() const;

    // Hot-reloading system
    /**
     * @brief Enable or disable file watching for hot-reload
     * @param enable True to enable hot-reload, false to disable
     */
    void enableHotReload(bool enable);
    
    /**
     * @brief Check for file changes and reload modified shaders
     */
    void checkForChanges();
    
    /**
     * @brief Set callback for shader reload events
     * @param callback Function called when shader is reloaded
     */
    void setReloadCallback(std::function<void(GLuint, const std::string&)> callback);

    // Shader introspection
    /**
     * @brief Get all uniform names in a shader program
     * @param program OpenGL program ID
     * @return Vector of uniform variable names
     */
    std::vector<std::string> getUniformNames(GLuint program) const;
    
    /**
     * @brief Get shader compilation log
     * @param program OpenGL program ID
     * @return Compilation log string
     */
    std::string getShaderLog(GLuint program) const;

    // Statistics and debugging
    /**
     * @brief Get shader compilation statistics
     * @return Map of statistic names to values
     */
    std::unordered_map<std::string, int> getStatistics() const;
    
    /**
     * @brief Get cache hit count
     * @return Number of cache hits
     */
    int getCacheHits() const;
    
    /**
     * @brief Get hot reload count
     * @return Number of hot reloads performed
     */
    int getHotReloads() const;
    
    /**
     * @brief Get total compilation count
     * @return Number of shaders compiled
     */
    int getCompilationCount() const;
    
    /**
     * @brief Clear all caches and force recompilation
     */
    void clearCaches();

private:
    struct Impl;                            ///< Forward declaration for PIMPL idiom
    std::unique_ptr<Impl> impl;             ///< Private implementation pointer

    // Helper functions
    /**
     * @brief Compile individual shader stage
     * @param source Shader source code
     * @param shaderType OpenGL shader type (GL_VERTEX_SHADER, etc.)
     * @return Compiled shader ID or 0 on failure
     */
    GLuint compileShader(const std::string& source, GLenum shaderType);
    
    /**
     * @brief Link compiled shaders into program
     * @param vertexShader Compiled vertex shader ID
     * @param fragmentShader Compiled fragment shader ID
     * @return Linked program ID or 0 on failure
     */
    GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader);
    
    /**
     * @brief Load file contents
     * @param path File path to load
     * @return File contents or empty string on failure
     */
    std::string loadFile(const std::string& path);
    
    /**
     * @brief Process #include directives recursively
     * @param source Shader source with includes
     * @return Processed source with includes resolved
     */
    std::string processIncludes(const std::string& source);
    
    /**
     * @brief Generate vertex shader for uber shader
     * @param options Uber shader generation options
     * @return Generated vertex shader source
     */
    std::string generateUberVertexShader(const UberShaderOptions& options);
    
    /**
     * @brief Generate fragment shader for uber shader
     * @param options Uber shader generation options
     * @return Generated fragment shader source
     */
    std::string generateUberFragmentShader(const UberShaderOptions& options);
};
