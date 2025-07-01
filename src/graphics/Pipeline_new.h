#pragma once

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>
#include <chrono>
#include <GL/glew.h>

// Forward declarations
class RenderContext;
class ShaderManager;
class PipelineGraph;

/**
 * @brief Pipeline - 그래픽스 파이프라인 관리
 * 
 * 책임:
 * - 노드 기반 파이프라인 구성 관리
 * - 렌더링 순서 관리
 * - 중간 결과물(FBO) 관리
 * - 파이프라인 기반 셰이더 생성 지시
 * 
 * 이 클래스는 PipelineGraph를 래핑하여 실제 렌더링 실행을 담당합니다.
 */
class Pipeline {
public:
    Pipeline();
    ~Pipeline();

    /**
     * @brief 파이프라인 초기화
     * @param context 렌더 컨텍스트
     * @param shaderMgr 셰이더 매니저
     * @return 초기화 성공 여부
     */
    bool initialize(RenderContext* context, ShaderManager* shaderMgr);

    /**
     * @brief 파이프라인 정리
     */
    void cleanup();

    // ====================================================
    // 노드 관리
    // ====================================================

    /**
     * @brief 새 노드 추가
     * @param type 노드 타입 (예: "noise", "blend", etc.)
     * @param parameters 노드 파라미터들
     * @return 생성된 노드 ID, 실패 시 -1
     */
    int addNode(const std::string& type, const std::unordered_map<std::string, std::string>& parameters = {});

    /**
     * @brief 노드 제거
     * @param nodeId 제거할 노드 ID
     * @return 제거 성공 여부
     */
    bool removeNode(int nodeId);

    /**
     * @brief 노드들 연결
     * @param fromNodeId 출력 노드 ID
     * @param fromPort 출력 포트 이름
     * @param toNodeId 입력 노드 ID
     * @param toPort 입력 포트 이름
     * @return 연결 성공 여부
     */
    bool connectNodes(int fromNodeId, const std::string& fromPort, int toNodeId, const std::string& toPort);

    /**
     * @brief 노드들 연결 해제
     * @param fromNodeId 출력 노드 ID
     * @param fromPort 출력 포트 이름
     * @param toNodeId 입력 노드 ID
     * @param toPort 입력 포트 이름
     * @return 연결 해제 성공 여부
     */
    bool disconnectNodes(int fromNodeId, const std::string& fromPort, int toNodeId, const std::string& toPort);

    /**
     * @brief 출력 노드 설정
     * @param nodeId 출력으로 사용할 노드 ID
     * @param port 출력 포트 이름
     * @return 설정 성공 여부
     */
    bool setOutput(int nodeId, const std::string& port = "output");

    // ====================================================
    // 실행 및 컴파일
    // ====================================================

    /**
     * @brief 파이프라인을 셰이더로 컴파일
     * @return 컴파일 성공 여부
     */
    bool compile();

    /**
     * @brief 파이프라인 실행 (렌더링)
     * @return 실행 성공 여부
     */
    bool execute();

    /**
     * @brief 파이프라인 유효성 검사
     * @return 유효성 여부
     */
    bool isValid() const;

    // ====================================================
    // 직렬화 및 저장
    // ====================================================

    /**
     * @brief 파이프라인을 문자열로 직렬화
     * @return 직렬화된 파이프라인 데이터
     */
    std::string serialize() const;

    /**
     * @brief 문자열에서 파이프라인 복원
     * @param data 직렬화된 파이프라인 데이터
     * @return 복원 성공 여부
     */
    bool deserialize(const std::string& data);

    // ====================================================
    // 그래프 접근
    // ====================================================

    /**
     * @brief 내부 파이프라인 그래프 반환 (읽기 전용)
     * @return 파이프라인 그래프 참조
     */
    const PipelineGraph& getGraph() const;

    /**
     * @brief 내부 파이프라인 그래프 반환 (쓰기 가능)
     * @return 파이프라인 그래프 참조
     */
    PipelineGraph& getGraph();

    // ====================================================
    // 디버그 및 모니터링
    // ====================================================

    /**
     * @brief 파이프라인 그래프 정보 출력
     */
    void printGraphInfo() const;

    /**
     * @brief 현재 컴파일된 프로그램 ID 반환
     * @return OpenGL 프로그램 ID
     */
    GLuint getCurrentProgram() const;

    /**
     * @brief 현재 실행 중인지 확인
     * @return 실행 중이면 true
     */
    bool isExecuting() const;

private:
    struct Impl;                                    ///< PIMPL 구현을 위한 전방 선언
    std::unique_ptr<Impl> impl;                     ///< 프라이빗 구현 포인터

    /**
     * @brief 현재 시간 반환 (초 단위)
     * @return 시작 시점으로부터의 경과 시간
     */
    double getCurrentTime() const;
};
