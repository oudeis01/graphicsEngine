#include "PipelineGraph.h"
#include "GeneratorModules.h"
#include "OperatorModules.h"
#include <sstream>
#include <algorithm>
#include <queue>
#include <set>
#include <iostream>

PipelineGraph::PipelineGraph() 
    : nextNodeId_(1), outputNodeId_(-1), outputPort_("output") {
}

int PipelineGraph::addNode(const std::string& moduleName, 
                          const std::unordered_map<std::string, std::string>& parameters) {
    auto module = ModuleFactory::createModule(moduleName);
    if (!module) {
        std::cerr << "Failed to create module: " << moduleName << std::endl;
        return -1;
    }
    
    int nodeId = nextNodeId_++;
    auto node = std::make_unique<PipelineNode>(nodeId, std::move(module));
    node->parameters = parameters;
    
    nodes_.push_back(std::move(node));
    
    std::cout << "Added node " << nodeId << " (" << moduleName << ")" << std::endl;
    return nodeId;
}

bool PipelineGraph::removeNode(int nodeId) {
    // 연결된 모든 커넥션 제거
    connections_.erase(
        std::remove_if(connections_.begin(), connections_.end(),
            [nodeId](const Connection& conn) {
                return conn.fromNodeId == nodeId || conn.toNodeId == nodeId;
            }),
        connections_.end()
    );
    
    // 노드 제거
    auto it = std::remove_if(nodes_.begin(), nodes_.end(),
        [nodeId](const std::unique_ptr<PipelineNode>& node) {
            return node->id == nodeId;
        });
    
    if (it != nodes_.end()) {
        nodes_.erase(it, nodes_.end());
        
        // 출력 노드였다면 리셋
        if (outputNodeId_ == nodeId) {
            outputNodeId_ = -1;
        }
        
        return true;
    }
    
    return false;
}

bool PipelineGraph::addConnection(int fromNodeId, const std::string& fromPort,
                                 int toNodeId, const std::string& toPort) {
    // 노드 존재 확인
    auto fromNode = findNode(fromNodeId);
    auto toNode = findNode(toNodeId);
    
    if (!fromNode || !toNode) {
        std::cerr << "Invalid node IDs for connection" << std::endl;
        return false;
    }
    
    // 포트 유효성 확인
    auto& fromOutputs = fromNode->module->getOutputPorts();
    auto& toInputs = toNode->module->getInputPorts();
    
    bool fromPortValid = std::any_of(fromOutputs.begin(), fromOutputs.end(),
        [&fromPort](const ModulePort& port) { return port.name == fromPort; });
    
    bool toPortValid = std::any_of(toInputs.begin(), toInputs.end(),
        [&toPort](const ModulePort& port) { return port.name == toPort; });
    
    if (!fromPortValid || !toPortValid) {
        std::cerr << "Invalid port names for connection" << std::endl;
        return false;
    }
    
    // 기존 연결 제거 (같은 입력 포트에는 하나의 연결만 가능)
    removeConnection(toNodeId, toPort);
    
    // 새 연결 추가
    connections_.push_back({fromNodeId, fromPort, toNodeId, toPort});
    
    std::cout << "Connected " << fromNodeId << "." << fromPort 
              << " -> " << toNodeId << "." << toPort << std::endl;
    return true;
}

bool PipelineGraph::removeConnection(int toNodeId, const std::string& toPort) {
    auto it = std::remove_if(connections_.begin(), connections_.end(),
        [toNodeId, &toPort](const Connection& conn) {
            return conn.toNodeId == toNodeId && conn.toPort == toPort;
        });
    
    if (it != connections_.end()) {
        connections_.erase(it, connections_.end());
        return true;
    }
    
    return false;
}

bool PipelineGraph::disconnect(int fromNodeId, const std::string& fromPort, int toNodeId, const std::string& toPort) {
    auto it = std::remove_if(connections_.begin(), connections_.end(),
        [fromNodeId, &fromPort, toNodeId, &toPort](const Connection& conn) {
            return conn.fromNodeId == fromNodeId && conn.fromPort == fromPort &&
                   conn.toNodeId == toNodeId && conn.toPort == toPort;
        });
    
    if (it != connections_.end()) {
        connections_.erase(it, connections_.end());
        std::cout << "Disconnected " << fromNodeId << "." << fromPort 
                  << " -> " << toNodeId << "." << toPort << std::endl;
        return true;
    }
    
    return false;
}

bool PipelineGraph::updateNodeParameters(int nodeId, 
                                        const std::unordered_map<std::string, std::string>& parameters) {
    auto node = findNode(nodeId);
    if (node) {
        node->parameters = parameters;
        return true;
    }
    return false;
}

void PipelineGraph::setOutputNode(int nodeId, const std::string& outputPort) {
    outputNodeId_ = nodeId;
    outputPort_ = outputPort;
}

PipelineGraph::ShaderCode PipelineGraph::generateUberShader() const {
    if (outputNodeId_ == -1) {
        return {"", "", {}};
    }
    
    return generateNodePreviewShader(outputNodeId_, outputPort_);
}

PipelineGraph::ShaderCode PipelineGraph::generateNodePreviewShader(int nodeId, const std::string& outputPort) const {
    ShaderCode result;
    
    // 종속성 수집 (토폴로지 순서)
    auto dependencies = collectDependencies(nodeId);
    
    // 필요한 includes 수집
    std::set<std::string> includeSet;
    for (int depNodeId : dependencies) {
        auto node = findNode(depNodeId);
        if (node) {
            auto nodeIncludes = node->module->getRequiredIncludes();
            includeSet.insert(nodeIncludes.begin(), nodeIncludes.end());
        }
    }
    result.requiredIncludes = std::vector<std::string>(includeSet.begin(), includeSet.end());
    
    // Vertex Shader 생성
    std::stringstream vs;
    vs << "#version 410 core\n\n";
    vs << "layout (location = 0) in vec2 aPos;\n";
    vs << "layout (location = 1) in vec2 aTexCoord;\n\n";
    vs << "out vec2 TexCoord;\n\n";
    vs << "void main() {\n";
    vs << "    gl_Position = vec4(aPos, 0.0, 1.0);\n";
    vs << "    TexCoord = aTexCoord;\n";
    vs << "}\n";
    result.vertexShader = vs.str();
    
    // Fragment Shader 생성
    std::stringstream fs;
    fs << "#version 410 core\n\n";
    
    // Includes 추가
    for (const auto& include : result.requiredIncludes) {
        fs << "#include \"" << include << "\"\n";
    }
    fs << "\n";
    
    // 기본 입출력
    fs << "in vec2 TexCoord;\n";
    fs << "out vec4 FragColor;\n\n";
    
    // 유니폼 변수들
    fs << "uniform float iTime;\n";
    fs << "uniform vec2 iResolution;\n\n";
    
    // 메인 함수 시작
    fs << "void main() {\n";
    fs << "    vec2 uv = TexCoord;\n\n";
    
    // 각 노드의 코드 생성
    std::unordered_map<std::string, std::string> variableMap;
    variableMap["uv"] = "uv";
    variableMap["time"] = "iTime";
    variableMap["resolution"] = "iResolution";
    
    for (int depNodeId : dependencies) {
        auto node = findNode(depNodeId);
        if (!node) continue;
        
        // 입력 변수 매핑 구성
        std::unordered_map<std::string, std::string> inputs;
        std::unordered_map<std::string, std::string> outputs;
        
        // 연결된 입력들 찾기
        for (const auto& conn : connections_) {
            if (conn.toNodeId == depNodeId) {
                std::string sourceVar = generateVariableName(conn.fromNodeId, conn.fromPort);
                inputs[conn.toPort] = variableMap.count(sourceVar) ? variableMap[sourceVar] : sourceVar;
            }
        }
        
        // 연결되지 않은 입력들에 기본값 사용
        for (const auto& port : node->module->getInputPorts()) {
            if (inputs.find(port.name) == inputs.end() && !port.defaultValue.empty()) {
                // 노드 파라미터에서 오버라이드된 값 확인
                if (node->parameters.count(port.name)) {
                    inputs[port.name] = node->parameters.at(port.name);
                } else {
                    inputs[port.name] = port.defaultValue;
                }
            }
        }
        
        // 출력 변수 생성
        for (const auto& port : node->module->getOutputPorts()) {
            std::string outputVar = generateVariableName(depNodeId, port.name);
            outputs[port.name] = outputVar;
            variableMap[outputVar] = outputVar;
        }
        
        // 노드 코드 생성
        fs << "    // Node " << depNodeId << " (" << node->module->getName() << ")\n";
        fs << node->module->generateGLSL(inputs, outputs);
        fs << "\n";
    }
    
    // 최종 출력
    std::string finalOutputVar = generateVariableName(nodeId, outputPort);
    if (variableMap.count(finalOutputVar)) {
        auto targetNode = findNode(nodeId);
        if (targetNode) {
            // 출력 타입에 따라 최종 색상 생성
            auto& outputPorts = targetNode->module->getOutputPorts();
            auto portIt = std::find_if(outputPorts.begin(), outputPorts.end(),
                [&outputPort](const ModulePort& port) { return port.name == outputPort; });
            
            if (portIt != outputPorts.end()) {
                switch (portIt->type) {
                    case DataType::FLOAT:
                        fs << "    FragColor = vec4(vec3(" << finalOutputVar << "), 1.0);\n";
                        break;
                    case DataType::VEC3:
                        fs << "    FragColor = vec4(" << finalOutputVar << ", 1.0);\n";
                        break;
                    case DataType::VEC4:
                        fs << "    FragColor = " << finalOutputVar << ";\n";
                        break;
                    default:
                        fs << "    FragColor = vec4(1.0, 0.0, 1.0, 1.0); // Error: unsupported type\n";
                        break;
                }
            }
        }
    } else {
        fs << "    FragColor = vec4(1.0, 0.0, 0.0, 1.0); // Error: missing output\n";
    }
    
    fs << "}\n";
    result.fragmentShader = fs.str();
    
    return result;
}

PipelineGraph::ValidationResult PipelineGraph::validateGraph() const {
    ValidationResult result;
    result.isValid = true;
    
    // 출력 노드 확인
    if (outputNodeId_ == -1) {
        result.errors.push_back("No output node specified");
        result.isValid = false;
    } else if (!findNode(outputNodeId_)) {
        result.errors.push_back("Output node does not exist");
        result.isValid = false;
    }
    
    // 순환 종속성 확인
    try {
        getTopologicalOrderIds();
    } catch (const std::runtime_error& e) {
        result.errors.push_back("Circular dependency detected");
        result.isValid = false;
    }
    
    // 연결되지 않은 필수 입력 확인
    for (const auto& node : nodes_) {
        for (const auto& port : node->module->getInputPorts()) {
            if (port.required) {
                bool connected = std::any_of(connections_.begin(), connections_.end(),
                    [nodeId = node->id, &port](const Connection& conn) {
                        return conn.toNodeId == nodeId && conn.toPort == port.name;
                    });
                
                if (!connected && port.defaultValue.empty()) {
                    result.warnings.push_back("Node " + std::to_string(node->id) + 
                                            " has unconnected required input: " + port.name);
                }
            }
        }
    }
    
    return result;
}

void PipelineGraph::clear() {
    nodes_.clear();
    connections_.clear();
    nextNodeId_ = 1;
    outputNodeId_ = -1;
    outputPort_ = "output";
}

PipelineNode* PipelineGraph::findNode(int nodeId) {
    auto it = std::find_if(nodes_.begin(), nodes_.end(),
        [nodeId](const std::unique_ptr<PipelineNode>& node) {
            return node->id == nodeId;
        });
    return (it != nodes_.end()) ? it->get() : nullptr;
}

const PipelineNode* PipelineGraph::findNode(int nodeId) const {
    auto it = std::find_if(nodes_.begin(), nodes_.end(),
        [nodeId](const std::unique_ptr<PipelineNode>& node) {
            return node->id == nodeId;
        });
    return (it != nodes_.end()) ? it->get() : nullptr;
}

std::vector<int> PipelineGraph::getTopologicalOrderIds() const {
    std::unordered_map<int, int> inDegree;
    std::unordered_map<int, std::vector<int>> adjList;
    
    // 그래프 구성
    for (const auto& node : nodes_) {
        inDegree[node->id] = 0;
        adjList[node->id] = {};
    }
    
    for (const auto& conn : connections_) {
        adjList[conn.fromNodeId].push_back(conn.toNodeId);
        inDegree[conn.toNodeId]++;
    }
    
    // Kahn's algorithm
    std::queue<int> queue;
    for (const auto& [nodeId, degree] : inDegree) {
        if (degree == 0) {
            queue.push(nodeId);
        }
    }
    
    std::vector<int> result;
    while (!queue.empty()) {
        int current = queue.front();
        queue.pop();
        result.push_back(current);
        
        for (int neighbor : adjList[current]) {
            inDegree[neighbor]--;
            if (inDegree[neighbor] == 0) {
                queue.push(neighbor);
            }
        }
    }
    
    if (result.size() != nodes_.size()) {
        throw std::runtime_error("Circular dependency detected in pipeline graph");
    }
    
    return result;
}

std::vector<int> PipelineGraph::collectDependencies(int nodeId) const {
    std::set<int> visited;
    std::vector<int> dependencies;
    
    std::function<void(int)> dfs = [&](int currentId) {
        if (visited.count(currentId)) return;
        visited.insert(currentId);
        
        // 현재 노드의 모든 입력 종속성 먼저 방문
        for (const auto& conn : connections_) {
            if (conn.toNodeId == currentId) {
                dfs(conn.fromNodeId);
            }
        }
        
        dependencies.push_back(currentId);
    };
    
    dfs(nodeId);
    return dependencies;
}

std::string PipelineGraph::generateVariableName(int nodeId, const std::string& portName) const {
    return "node" + std::to_string(nodeId) + "_" + portName;
}

// DSL 파싱은 나중에 구현 (현재는 스텁)
bool PipelineGraph::fromDSL(const std::string& dslCode) {
    // TODO: AngelScript DSL 파싱 구현
    return false;
}

std::string PipelineGraph::toDSL() const {
    // TODO: DSL 생성 구현
    return "";
}

// ====================================================
// ShaderManager와의 통합을 위한 추가 메서드들 구현
// ====================================================

std::unique_ptr<PipelineGraph> PipelineGraph::extractSubgraphTo(int nodeId) const {
    auto subGraph = std::make_unique<PipelineGraph>();
    
    // 해당 노드까지의 종속성 수집
    auto dependencies = collectDependencies(nodeId);
    
    // 노드 ID 매핑 테이블 (원본 ID -> 새 ID)
    std::unordered_map<int, int> idMapping;
    
    // 종속성 노드들을 서브그래프에 추가
    for (int originalId : dependencies) {
        const PipelineNode* originalNode = findNode(originalId);
        if (originalNode) {
            // 모듈 복제 (간단한 구현을 위해 모듈 이름으로 새로 생성)
            std::string moduleName = originalNode->module->getName();
            int newId = subGraph->addNode(moduleName, originalNode->parameters);
            idMapping[originalId] = newId;
            
            // 목표 노드가 출력 노드라면 설정
            if (originalId == nodeId) {
                subGraph->setOutput(newId, outputPort_);
            }
        }
    }
    
    // 연결 정보 복사 (새로운 ID로 매핑)
    for (const auto& conn : connections_) {
        auto fromIt = idMapping.find(conn.fromNodeId);
        auto toIt = idMapping.find(conn.toNodeId);
        
        if (fromIt != idMapping.end() && toIt != idMapping.end()) {
            subGraph->connect(fromIt->second, conn.fromPort, toIt->second, conn.toPort);
        }
    }
    
    return subGraph;
}

std::set<std::string> PipelineGraph::getRequiredLygiaModules() const {
    std::set<std::string> modules;
    
    for (const auto& node : nodes_) {
        if (node->module) {
            // 각 모듈에서 요구하는 LYGIA 모듈들 수집
            auto moduleRequirements = node->module->getRequiredLygiaModules();
            modules.insert(moduleRequirements.begin(), moduleRequirements.end());
        }
    }
    
    return modules;
}

std::vector<PipelineGraph::Node> PipelineGraph::getTopologicalOrder() const {
    std::vector<Node> result;
    auto sortedIds = getTopologicalOrderIds();
    
    for (int id : sortedIds) {
        const PipelineNode* pipelineNode = findNode(id);
        if (pipelineNode) {
            result.emplace_back(pipelineNode);
        }
    }
    
    return result;
}

std::vector<PipelineGraph::Node> PipelineGraph::getDependenciesOf(int nodeId) const {
    std::vector<Node> result;
    auto dependencyIds = collectDependencies(nodeId);
    
    for (int id : dependencyIds) {
        const PipelineNode* pipelineNode = findNode(id);
        if (pipelineNode) {
            result.emplace_back(pipelineNode);
        }
    }
    
    return result;
}

std::vector<PipelineGraph::Node> PipelineGraph::getOutputNodes() const {
    std::vector<Node> result;
    
    if (outputNodeId_ != -1) {
        const PipelineNode* outputNode = findNode(outputNodeId_);
        if (outputNode) {
            result.emplace_back(outputNode);
        }
    }
    
    return result;
}

PipelineGraph::Node PipelineGraph::getNode(int nodeId) const {
    const PipelineNode* pipelineNode = findNode(nodeId);
    if (pipelineNode) {
        return Node(pipelineNode);
    }
    
    // 더미 노드 반환 (오류 처리)
    static PipelineNode dummyNode(-1, nullptr);
    return Node(&dummyNode);
}
