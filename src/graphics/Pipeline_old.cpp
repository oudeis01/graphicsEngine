#include "Pipeline.h"
#include "RenderContext.h"
#include "ShaderManager.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <queue>

struct Pipeline::Impl {
    std::vector<PipelineNode> nodes;
    std::string outputNodeId;
    std::vector<std::string> errors;
    
    // 캐시된 실행 순서
    mutable std::vector<std::string> cachedExecutionOrder;
    mutable bool executionOrderDirty = true;
};

Pipeline::Pipeline() : impl(std::make_unique<Impl>()) {}

Pipeline::~Pipeline() = default;

void Pipeline::addNode(const PipelineNode& node) {
    // 기존 노드가 있으면 교체
    auto it = std::find_if(impl->nodes.begin(), impl->nodes.end(),
        [&](const PipelineNode& n) { return n.id == node.id; });
    
    if (it != impl->nodes.end()) {
        *it = node;
    } else {
        impl->nodes.push_back(node);
    }
    
    impl->executionOrderDirty = true;
}

void Pipeline::removeNode(const std::string& nodeId) {
    impl->nodes.erase(
        std::remove_if(impl->nodes.begin(), impl->nodes.end(),
            [&](const PipelineNode& n) { return n.id == nodeId; }),
        impl->nodes.end());
    
    // 다른 노드들의 연결에서도 제거
    for (auto& node : impl->nodes) {
        node.inputs.erase(
            std::remove(node.inputs.begin(), node.inputs.end(), nodeId),
            node.inputs.end());
    }
    
    if (impl->outputNodeId == nodeId) {
        impl->outputNodeId.clear();
    }
    
    impl->executionOrderDirty = true;
}

void Pipeline::connectNodes(const std::string& fromNode, const std::string& toNode) {
    auto toNodePtr = getNode(toNode);
    if (toNodePtr) {
        if (std::find(toNodePtr->inputs.begin(), toNodePtr->inputs.end(), fromNode) 
            == toNodePtr->inputs.end()) {
            toNodePtr->inputs.push_back(fromNode);
            impl->executionOrderDirty = true;
        }
    }
}

void Pipeline::disconnectNodes(const std::string& fromNode, const std::string& toNode) {
    auto toNodePtr = getNode(toNode);
    if (toNodePtr) {
        toNodePtr->inputs.erase(
            std::remove(toNodePtr->inputs.begin(), toNodePtr->inputs.end(), fromNode),
            toNodePtr->inputs.end());
        impl->executionOrderDirty = true;
    }
}

void Pipeline::setOutputNode(const std::string& nodeId) {
    impl->outputNodeId = nodeId;
}

std::string Pipeline::getOutputNode() const {
    return impl->outputNodeId;
}

bool Pipeline::validate() const {
    impl->errors.clear();
    
    if (impl->nodes.empty()) {
        impl->errors.push_back("Pipeline is empty");
        return false;
    }
    
    if (impl->outputNodeId.empty()) {
        impl->errors.push_back("No output node specified");
        return false;
    }
    
    // 출력 노드 존재 확인
    auto outputNode = std::find_if(impl->nodes.begin(), impl->nodes.end(),
        [&](const PipelineNode& n) { return n.id == impl->outputNodeId; });
    
    if (outputNode == impl->nodes.end()) {
        impl->errors.push_back("Output node not found: " + impl->outputNodeId);
        return false;
    }
    
    // 순환 참조 검사
    try {
        getExecutionOrder();
    } catch (const std::exception& e) {
        impl->errors.push_back("Circular dependency detected");
        return false;
    }
    
    // 존재하지 않는 입력 노드 검사
    for (const auto& node : impl->nodes) {
        for (const auto& input : node.inputs) {
            auto inputNode = std::find_if(impl->nodes.begin(), impl->nodes.end(),
                [&](const PipelineNode& n) { return n.id == input; });
            if (inputNode == impl->nodes.end()) {
                impl->errors.push_back("Input node not found: " + input + " (required by " + node.id + ")");
            }
        }
    }
    
    return impl->errors.empty();
}

std::vector<std::string> Pipeline::getErrors() const {
    return impl->errors;
}

void Pipeline::render(RenderContext& renderContext, ShaderManager& shaderManager) {
    if (!validate()) {
        std::cerr << "Pipeline validation failed:" << std::endl;
        for (const auto& error : impl->errors) {
            std::cerr << "  " << error << std::endl;
        }
        return;
    }
    
    auto executionOrder = getExecutionOrder();
    
    // 각 노드를 순서대로 실행
    for (const auto& nodeId : executionOrder) {
        auto node = getNode(nodeId);
        if (node) {
            renderNode(*node, renderContext, shaderManager);
        }
    }
}

const std::vector<PipelineNode>& Pipeline::getNodes() const {
    return impl->nodes;
}

PipelineNode* Pipeline::getNode(const std::string& nodeId) {
    auto it = std::find_if(impl->nodes.begin(), impl->nodes.end(),
        [&](const PipelineNode& n) { return n.id == nodeId; });
    return it != impl->nodes.end() ? &(*it) : nullptr;
}

std::string Pipeline::serialize() const {
    std::stringstream ss;
    
    for (const auto& node : impl->nodes) {
        switch (node.type) {
            case NodeType::Generator:
                ss << "gen " << node.id << "=" << node.operation << "(";
                for (size_t i = 0; i < node.parameters.size(); ++i) {
                    if (i > 0) ss << ",";
                    ss << node.parameters[i];
                }
                ss << ");\n";
                break;
                
            case NodeType::Operator:
                ss << node.id << "=" << node.operation << "(";
                for (size_t i = 0; i < node.inputs.size(); ++i) {
                    if (i > 0) ss << ",";
                    ss << node.inputs[i];
                }
                for (const auto& param : node.parameters) {
                    ss << "," << param;
                }
                ss << ");\n";
                break;
                
            case NodeType::Output:
                ss << "output(" << node.inputs[0] << "," << node.parameters[0] << ");\n";
                break;
        }
    }
    
    return ss.str();
}

bool Pipeline::deserialize(const std::string& data) {
    // TODO: DSL 파싱 구현
    impl->nodes.clear();
    impl->outputNodeId.clear();
    impl->executionOrderDirty = true;
    
    std::istringstream stream(data);
    std::string line;
    
    while (std::getline(stream, line)) {
        // 간단한 파싱 예제 (실제로는 더 정교한 파서 필요)
        if (line.find("gen ") == 0) {
            // 생성기 노드 파싱
            // 예: gen n=noise();
            size_t eq = line.find('=');
            size_t paren = line.find('(');
            if (eq != std::string::npos && paren != std::string::npos) {
                PipelineNode node;
                node.id = line.substr(4, eq - 4);
                node.type = NodeType::Generator;
                node.operation = line.substr(eq + 1, paren - eq - 1);
                addNode(node);
            }
        } else if (line.find("output(") == 0) {
            // 출력 노드 파싱
            // 예: output(n,0);
            size_t start = line.find('(') + 1;
            size_t comma = line.find(',', start);
            size_t end = line.find(')', comma);
            if (comma != std::string::npos && end != std::string::npos) {
                std::string inputId = line.substr(start, comma - start);
                std::string channel = line.substr(comma + 1, end - comma - 1);
                
                PipelineNode node;
                node.id = "output";
                node.type = NodeType::Output;
                node.operation = "output";
                node.inputs.push_back(inputId);
                node.parameters.push_back(channel);
                addNode(node);
                setOutputNode("output");
            }
        }
    }
    
    return validate();
}

std::shared_ptr<Pipeline> Pipeline::fromDescription(const std::string& description) {
    auto pipeline = std::make_shared<Pipeline>();
    if (pipeline->deserialize(description)) {
        return pipeline;
    }
    return nullptr;
}

std::vector<std::string> Pipeline::getExecutionOrder() const {
    if (!impl->executionOrderDirty && !impl->cachedExecutionOrder.empty()) {
        return impl->cachedExecutionOrder;
    }
    
    std::vector<std::string> result;
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> visiting;
    
    std::function<void(const std::string&)> visit = [&](const std::string& nodeId) {
        if (visiting.count(nodeId)) {
            throw std::runtime_error("Circular dependency detected");
        }
        if (visited.count(nodeId)) {
            return;
        }
        
        visiting.insert(nodeId);
        
        auto node = std::find_if(impl->nodes.begin(), impl->nodes.end(),
            [&](const PipelineNode& n) { return n.id == nodeId; });
        
        if (node != impl->nodes.end()) {
            for (const auto& input : node->inputs) {
                visit(input);
            }
        }
        
        visiting.erase(nodeId);
        visited.insert(nodeId);
        result.push_back(nodeId);
    };
    
    if (!impl->outputNodeId.empty()) {
        visit(impl->outputNodeId);
    }
    
    impl->cachedExecutionOrder = result;
    impl->executionOrderDirty = false;
    
    return result;
}

void Pipeline::renderNode(const PipelineNode& node, RenderContext& renderContext, ShaderManager& shaderManager) {
    // 기본 셰이더 사용 (임시)
    shaderManager.useShader(shaderManager.getDefaultShader());
    
    // 유니폼 설정
    int width, height;
    renderContext.getViewport(width, height);
    shaderManager.setUniform("iResolution", static_cast<float>(width), static_cast<float>(height));
    shaderManager.setUniform("iTime", renderContext.getTime());
    
    // 풀스크린 쿼드 렌더링
    renderContext.renderFullscreenQuad();
}

std::string Pipeline::generateShaderForNode(const PipelineNode& node) const {
    // TODO: 노드 기반 셰이더 생성 로직 구현
    std::stringstream ss;
    
    switch (node.type) {
        case NodeType::Generator:
            if (node.operation == "noise") {
                ss << "// Noise generator\n";
                ss << "vec3 " << node.id << "_output = vec3(noise(uv));\n";
            } else if (node.operation == "voronoi") {
                ss << "// Voronoi generator\n"; 
                ss << "vec3 " << node.id << "_output = vec3(voronoi(uv));\n";
            }
            break;
            
        case NodeType::Operator:
            if (node.operation == "multiply") {
                if (node.inputs.size() >= 2) {
                    ss << "// Multiply operator\n";
                    ss << "vec3 " << node.id << "_output = " << node.inputs[0] << "_output * " << node.inputs[1] << "_output;\n";
                }
            }
            break;
            
        case NodeType::Output:
            ss << "// Output\n";
            ss << "FragColor = vec4(" << node.inputs[0] << "_output, 1.0);\n";
            break;
    }
    
    return ss.str();
}
