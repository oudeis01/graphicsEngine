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
    
    /**
     * @brief vec3 유니폼 값 설정
     * @param name 유니폼 변수 이름
     * @param x X 컴포넌트
     * @param y Y 컴포넌트
     * @param z Z 컴포넌트
     */
    void setUniform(const std::string& name, float x, float y, float z);
    
    /**
     * @brief vec4 유니폼 값 설정
     * @param name 유니폼 변수 이름
     * @param x X 컴포넌트
     * @param y Y 컴포넌트
     * @param z Z 컴포넌트
     * @param w W 컴포넌트
     */
    void setUniform(const std::string& name, float x, float y, float z, float w);

    // LYGIA 모듈 관리
    /**
     * @brief LYGIA 모듈 로딩
     * @param moduleName 모듈 이름 (예: "generative/snoise.glsl")
     * @return 모듈 소스 코드
     */
    std::string loadLygiaModule(const std::string& moduleName);
    
    /**
     * @brief 사용 가능한 모듈 목록 반환
     * @return 모듈 이름들의 벡터
     */
    std::vector<std::string> getAvailableModules() const;

    // Hot-reload 및 감시
    /**
     * @brief Hot-reload 기능 활성화/비활성화
     * @param enable Hot-reload 활성화 여부
     */
    void enableHotReload(bool enable);
    
    /**
     * @brief 파일 변경사항 확인 및 reload 수행
     */
    void checkForChanges();
    
    /**
     * @brief Reload 콜백 함수 설정
     * @param callback 콜백 함수 (program ID, 변경된 파일 경로)
     */
    void setReloadCallback(std::function<void(GLuint, const std::string&)> callback);

    // 통계 및 모니터링
    /**
     * @brief 컴파일 통계 반환
     * @return 통계 이름과 값의 매핑
     */
    std::unordered_map<std::string, int> getStatistics() const;
    
    /**
     * @brief 캐시 히트 수 반환
     * @return 캐시 히트 횟수
     */
    int getCacheHits() const;
    
    /**
     * @brief Hot reload 수 반환
     * @return Hot reload 수행 횟수
     */
    int getHotReloads() const;
    
    /**
     * @brief 총 컴파일 수 반환
     * @return 셰이더 컴파일 횟수
     */
    int getCompilationCount() const;
    
    /**
     * @brief 모든 캐시 지우기 및 강제 재컴파일
     */
    void clearCaches();

private:
    struct Impl;                            ///< PIMPL 구현을 위한 전방 선언
    std::unique_ptr<Impl> impl;             ///< 프라이빗 구현 포인터

    // 헬퍼 함수들
    /**
     * @brief 개별 셰이더 스테이지 컴파일
     * @param source 셰이더 소스 코드
     * @param shaderType OpenGL 셰이더 타입 (GL_VERTEX_SHADER 등)
     * @return 컴파일된 셰이더 ID 또는 실패 시 0
     */
    GLuint compileShader(const std::string& source, GLenum shaderType);
    
    /**
     * @brief 컴파일된 셰이더들을 프로그램으로 링크
     * @param vertexShader 컴파일된 버텍스 셰이더 ID
     * @param fragmentShader 컴파일된 프래그먼트 셰이더 ID
     * @return 링크된 프로그램 ID 또는 실패 시 0
     */
    GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader);
    
    /**
     * @brief 파일 내용 로딩
     * @param path 로딩할 파일 경로
     * @return 파일 내용 또는 실패 시 빈 문자열
     */
    std::string loadFile(const std::string& path);
    
    /**
     * @brief #include 지시문 재귀적 처리
     * @param source include가 포함된 셰이더 소스
     * @return include가 해결된 소스
     */
    std::string processIncludes(const std::string& source);
    
    /**
     * @brief 풀스크린 쿼드용 VAO 생성
     * @return VAO ID
     */
    GLuint createFullscreenQuadVAO();

    // ====================================================
    // 그래프 관련 헬퍼 함수들
    // ====================================================

    /**
     * @brief 그래프 기반 캐시 키 생성
     * @param graph 파이프라인 그래프
     * @return 고유 캐시 키 문자열
     */
    std::string generateGraphCacheKey(const PipelineGraph& graph);

    /**
     * @brief 그래프로부터 프래그먼트 셰이더 생성
     * @param graph 파이프라인 그래프
     * @return 생성된 프래그먼트 셰이더 코드
     */
    std::string generateFragmentShaderFromGraph(const PipelineGraph& graph);

    /**
     * @brief 노드 미리보기용 프래그먼트 셰이더 생성
     * @param graph 파이프라인 그래프
     * @param nodeId 대상 노드 ID
     * @param outputPort 출력 포트 이름
     * @return 생성된 프래그먼트 셰이더 코드
     */
    std::string generateNodePreviewFragmentShader(const PipelineGraph& graph, int nodeId, const std::string& outputPort);

    /**
     * @brief 노드 함수 코드 생성
     * @param node 노드 정보
     * @return 생성된 GLSL 함수 코드
     */
    std::string generateNodeFunction(const PipelineGraph::Node& node);

    /**
     * @brief 노드 호출 코드 생성
     * @param node 노드 정보
     * @return 생성된 GLSL 호출 코드
     */
    std::string generateNodeCall(const PipelineGraph::Node& node);

    /**
     * @brief 노드 입력 파라미터 생성
     * @param node 노드 정보
     * @return 생성된 입력 파라미터 문자열
     */
    std::string generateNodeInputs(const PipelineGraph::Node& node);

    /**
     * @brief 그래프 종속성 추적 (hot-reload용)
     * @param graph 파이프라인 그래프
     * @param program 프로그램 ID
     */
    void trackGraphDependencies(const PipelineGraph& graph, GLuint program);

    /**
     * @brief 노드별 유니폼 설정
     * @param graph 파이프라인 그래프
     * @param nodeId 노드 ID
     */
    void setNodeUniforms(const PipelineGraph& graph, int nodeId);
};
