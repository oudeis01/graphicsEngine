#pragma once

#include "PipelineModule.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <set>

/**
 * @brief 파이프라인 노드 간의 연결 정보
 */
struct Connection {
    int fromNodeId;         ///< 출력 노드 ID
    std::string fromPort;   ///< 출력 포트 이름
    int toNodeId;           ///< 입력 노드 ID
    std::string toPort;     ///< 입력 포트 이름
};

/**
 * @brief 파이프라인 그래프의 단일 노드
 */
struct PipelineNode {
    int id;                                             ///< 노드 고유 ID
    std::unique_ptr<PipelineModule> module;             ///< 모듈 인스턴스
    std::unordered_map<std::string, std::string> parameters; ///< 노드별 파라미터 오버라이드
    
    PipelineNode(int nodeId, std::unique_ptr<PipelineModule> mod)
        : id(nodeId), module(std::move(mod)) {}
};

/**
 * @brief 동적 파이프라인 그래프 관리 클래스
 * 
 * 노드들과 연결 정보를 관리하고, 이를 기반으로 우버 셰이더를 생성합니다.
 * 각 노드는 중간 FBO 렌더링을 위한 개별 함수로 컴파일됩니다.
 */
class PipelineGraph {
public:
    /**
     * @brief 노드 래퍼 클래스 (ShaderManager와의 인터페이스용)
     */
    class Node {
    public:
        Node(const PipelineNode* pipelineNode) : node_(pipelineNode) {}
        
        int getId() const { return node_->id; }
        std::string getType() const { return node_->module ? node_->module->getName() : "unknown"; }
        std::string getName() const { return "node_" + std::to_string(node_->id); }
        
        const std::unordered_map<std::string, std::string>& getParameters() const { 
            return node_->parameters; 
        }
        
    private:
        const PipelineNode* node_;
    };

    PipelineGraph();
    ~PipelineGraph() = default;
    
    // 복사 생성자와 복사 할당 연산자 삭제 (unique_ptr 때문에)
    PipelineGraph(const PipelineGraph&) = delete;
    PipelineGraph& operator=(const PipelineGraph&) = delete;
    
    // 이동 생성자와 이동 할당 연산자는 허용
    PipelineGraph(PipelineGraph&&) = default;
    PipelineGraph& operator=(PipelineGraph&&) = default;

    /**
     * @brief 새 노드 추가
     * @param moduleName 모듈 이름
     * @param parameters 노드별 파라미터
     * @return 생성된 노드 ID, 실패 시 -1
     */
    int addNode(const std::string& moduleName, 
                const std::unordered_map<std::string, std::string>& parameters = {});

    /**
     * @brief 노드 제거
     * @param nodeId 제거할 노드 ID
     * @return 성공 여부
     */
    bool removeNode(int nodeId);

    /**
     * @brief 노드 간 연결 추가
     * @param fromNodeId 출력 노드 ID
     * @param fromPort 출력 포트 이름
     * @param toNodeId 입력 노드 ID
     * @param toPort 입력 포트 이름
     * @return 성공 여부
     */
    bool addConnection(int fromNodeId, const std::string& fromPort,
                      int toNodeId, const std::string& toPort);

    /**
     * @brief 연결 제거
     * @param toNodeId 입력 노드 ID
     * @param toPort 입력 포트 이름
     * @return 성공 여부
     */
    bool removeConnection(int toNodeId, const std::string& toPort);

    /**
     * @brief 노드 간 연결 (Pipeline 호환성)
     * @param fromNodeId 출력 노드 ID
     * @param fromPort 출력 포트 이름
     * @param toNodeId 입력 노드 ID
     * @param toPort 입력 포트 이름
     * @return 성공 여부
     */
    bool connect(int fromNodeId, const std::string& fromPort, int toNodeId, const std::string& toPort) {
        return addConnection(fromNodeId, fromPort, toNodeId, toPort);
    }

    /**
     * @brief 연결 해제 (Pipeline 호환성)
     * @param fromNodeId 출력 노드 ID
     * @param fromPort 출력 포트 이름
     * @param toNodeId 입력 노드 ID
     * @param toPort 입력 포트 이름
     * @return 성공 여부
     */
    bool disconnect(int fromNodeId, const std::string& fromPort, int toNodeId, const std::string& toPort);

    /**
     * @brief 출력 설정 (Pipeline 호환성)
     * @param nodeId 출력 노드 ID
     * @param port 출력 포트 이름
     * @return 성공 여부
     */
    bool setOutput(int nodeId, const std::string& port = "output") {
        setOutputNode(nodeId, port);
        return true;
    }

    /**
     * @brief 노드 파라미터 업데이트
     * @param nodeId 노드 ID
     * @param parameters 새 파라미터들
     * @return 성공 여부
     */
    bool updateNodeParameters(int nodeId, const std::unordered_map<std::string, std::string>& parameters);

    /**
     * @brief 출력 노드 설정
     * @param nodeId 최종 출력을 담당할 노드 ID
     * @param outputPort 출력 포트 이름
     */
    void setOutputNode(int nodeId, const std::string& outputPort = "output");

    /**
     * @brief 우버 셰이더 생성
     * @return 생성된 GLSL 셰이더 코드 (vertex + fragment)
     */
    struct ShaderCode {
        std::string vertexShader;
        std::string fragmentShader;
        std::vector<std::string> requiredIncludes;
    };
    ShaderCode generateUberShader() const;

    /**
     * @brief 특정 노드의 출력을 위한 셰이더 생성 (노드 에디터 미리보기용)
     * @param nodeId 미리보기할 노드 ID
     * @param outputPort 출력 포트 이름
     * @return 해당 노드까지의 셰이더 코드
     */
    ShaderCode generateNodePreviewShader(int nodeId, const std::string& outputPort = "output") const;

    /**
     * @brief 그래프 유효성 검사
     * @return 검사 결과와 오류 메시지
     */
    struct ValidationResult {
        bool isValid;
        std::vector<std::string> errors;
        std::vector<std::string> warnings;
    };
    ValidationResult validateGraph() const;

    /**
     * @brief 그래프 정보 반환
     */
    const std::vector<std::unique_ptr<PipelineNode>>& getNodes() const { return nodes_; }
    const std::vector<Connection>& getConnections() const { return connections_; }
    int getOutputNodeId() const { return outputNodeId_; }
    const std::string& getOutputPort() const { return outputPort_; }

    // ====================================================
    // ShaderManager와의 통합을 위한 추가 메서드들
    // ====================================================

    /**
     * @brief 특정 노드까지의 서브그래프 추출
     * @param nodeId 대상 노드 ID
     * @return 해당 노드까지의 종속성만 포함하는 서브그래프
     */
    std::unique_ptr<PipelineGraph> extractSubgraphTo(int nodeId) const;

    /**
     * @brief 그래프에서 필요한 LYGIA 모듈들 수집
     * @return 필요한 모듈 이름들의 집합
     */
    std::set<std::string> getRequiredLygiaModules() const;

    /**
     * @brief 토폴로지 순서로 정렬된 노드들 반환 (ShaderManager용)
     * @return 실행 순서대로 정렬된 노드들
     */
    std::vector<Node> getTopologicalOrder() const;

    /**
     * @brief 특정 노드의 종속성들 반환
     * @param nodeId 대상 노드 ID
     * @return 종속성 노드들
     */
    std::vector<Node> getDependenciesOf(int nodeId) const;

    /**
     * @brief 출력 노드들 반환
     * @return 출력 노드들의 리스트
     */
    std::vector<Node> getOutputNodes() const;

    /**
     * @brief 노드 ID로 노드 반환
     * @param nodeId 노드 ID
     * @return 노드 객체
     */
    Node getNode(int nodeId) const;

    /**
     * @brief 그래프 초기화
     */
    void clear();

    /**
     * @brief DSL 문자열로부터 그래프 생성
     * @param dslCode DSL 코드 문자열
     * @return 파싱 성공 여부
     */
    bool fromDSL(const std::string& dslCode);

    /**
     * @brief 그래프를 DSL 문자열로 변환
     * @return DSL 코드 문자열
     */
    std::string toDSL() const;

private:
    std::vector<std::unique_ptr<PipelineNode>> nodes_;  ///< 노드들
    std::vector<Connection> connections_;               ///< 연결들
    int nextNodeId_;                                   ///< 다음 노드 ID
    int outputNodeId_;                                 ///< 출력 노드 ID
    std::string outputPort_;                           ///< 출력 포트 이름

    /**
     * @brief 노드 ID로 노드 찾기
     * @param nodeId 노드 ID
     * @return 노드 포인터 또는 nullptr
     */
    PipelineNode* findNode(int nodeId);
    const PipelineNode* findNode(int nodeId) const;

    /**
     * @brief 토폴로지 정렬된 노드 ID 순서 반환 (내부 구현용)
     * @return 정렬된 노드 ID들
     */
    std::vector<int> getTopologicalOrderIds() const;

    /**
     * @brief 특정 노드까지의 종속성 수집
     * @param nodeId 대상 노드 ID
     * @return 종속성 노드들 (토폴로지 순서)
     */
    std::vector<int> collectDependencies(int nodeId) const;

    /**
     * @brief 변수 이름 생성
     * @param nodeId 노드 ID
     * @param portName 포트 이름
     * @return 고유한 변수 이름
     */
    std::string generateVariableName(int nodeId, const std::string& portName) const;
};
