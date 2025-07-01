#pragma once

#include <vector>
#include <string>
#include <memory>
#include <functional>

class RenderContext;
class ShaderManager;

/**
 * Pipeline - 그래픽스 파이프라인 관리
 * 
 * 역할:
 * - 노드 기반 파이프라인 구성 관리
 * - 렌더링 순서 관리
 * - 중간 결과물(FBO) 관리
 * - 파이프라인 기반 셰이더 생성 지시
 */

// Forward declarations (실제 정의는 PipelineGraph.h에 있음)
struct PipelineNode;
class PipelineGraph;

class Pipeline {
public:
    Pipeline();
    ~Pipeline();

    // 노드 관리
    void addNode(const PipelineNode& node);
    void removeNode(const std::string& nodeId);
    void connectNodes(const std::string& fromNode, const std::string& toNode);
    void disconnectNodes(const std::string& fromNode, const std::string& toNode);
    
    // 파이프라인 설정
    void setOutputNode(const std::string& nodeId);
    std::string getOutputNode() const;
    
    // 파이프라인 검증
    bool validate() const;
    std::vector<std::string> getErrors() const;
    
    // 렌더링
    void render(RenderContext& renderContext, ShaderManager& shaderManager);
    
    // 파이프라인 정보
    const std::vector<PipelineNode>& getNodes() const;
    PipelineNode* getNode(const std::string& nodeId);
    
    // 직렬화 (스크립트 저장/로드용)
    std::string serialize() const;
    bool deserialize(const std::string& data);
    
    // 파이프라인 설명으로부터 생성 (DSL 파싱용)
    static std::shared_ptr<Pipeline> fromDescription(const std::string& description);

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
    
    // 헬퍼 함수들
    std::vector<std::string> getExecutionOrder() const;
    void renderNode(const PipelineNode& node, RenderContext& renderContext, ShaderManager& shaderManager);
    std::string generateShaderForNode(const PipelineNode& node) const;
};
