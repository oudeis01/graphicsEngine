#include "NodeGraph.h"
#include <sstream>
#include <algorithm>
#include <stdexcept>

namespace gfx {

// Parameter implementation
Parameter::Parameter(const std::string& name, osc::ParameterType type) 
    : name_(name), type_(type) {
    // Initialize data to zero
    data_.vec_values[0] = data_.vec_values[1] = data_.vec_values[2] = data_.vec_values[3] = 0.0f;
}

void Parameter::setValue(int value) {
    if (type_ != osc::ParameterType::INT) {
        throw std::runtime_error("Parameter type mismatch: expected int");
    }
    data_.int_value = value;
}

void Parameter::setValue(float value) {
    if (type_ != osc::ParameterType::FLOAT) {
        throw std::runtime_error("Parameter type mismatch: expected float");
    }
    data_.float_value = value;
}

void Parameter::setValue(const std::string& value) {
    if (type_ != osc::ParameterType::STRING) {
        throw std::runtime_error("Parameter type mismatch: expected string");
    }
    string_value_ = value;
}

void Parameter::setValue(bool value) {
    if (type_ != osc::ParameterType::BOOL) {
        throw std::runtime_error("Parameter type mismatch: expected bool");
    }
    data_.bool_value = value;
}

void Parameter::setValue(float x, float y) {
    if (type_ != osc::ParameterType::VEC2) {
        throw std::runtime_error("Parameter type mismatch: expected vec2");
    }
    data_.vec_values[0] = x;
    data_.vec_values[1] = y;
}

void Parameter::setValue(float x, float y, float z) {
    if (type_ != osc::ParameterType::VEC3) {
        throw std::runtime_error("Parameter type mismatch: expected vec3");
    }
    data_.vec_values[0] = x;
    data_.vec_values[1] = y;
    data_.vec_values[2] = z;
}

void Parameter::setValue(float x, float y, float z, float w) {
    if (type_ != osc::ParameterType::VEC4 && type_ != osc::ParameterType::COLOR) {
        throw std::runtime_error("Parameter type mismatch: expected vec4 or color");
    }
    data_.vec_values[0] = x;
    data_.vec_values[1] = y;
    data_.vec_values[2] = z;
    data_.vec_values[3] = w;
}

int Parameter::getIntValue() const {
    if (type_ != osc::ParameterType::INT) {
        throw std::runtime_error("Parameter type mismatch: not an int");
    }
    return data_.int_value;
}

float Parameter::getFloatValue() const {
    if (type_ != osc::ParameterType::FLOAT) {
        throw std::runtime_error("Parameter type mismatch: not a float");
    }
    return data_.float_value;
}

const std::string& Parameter::getStringValue() const {
    if (type_ != osc::ParameterType::STRING) {
        throw std::runtime_error("Parameter type mismatch: not a string");
    }
    return string_value_;
}

bool Parameter::getBoolValue() const {
    if (type_ != osc::ParameterType::BOOL) {
        throw std::runtime_error("Parameter type mismatch: not a bool");
    }
    return data_.bool_value;
}

void Parameter::getVec2Value(float& x, float& y) const {
    if (type_ != osc::ParameterType::VEC2) {
        throw std::runtime_error("Parameter type mismatch: not a vec2");
    }
    x = data_.vec_values[0];
    y = data_.vec_values[1];
}

void Parameter::getVec3Value(float& x, float& y, float& z) const {
    if (type_ != osc::ParameterType::VEC3) {
        throw std::runtime_error("Parameter type mismatch: not a vec3");
    }
    x = data_.vec_values[0];
    y = data_.vec_values[1];
    z = data_.vec_values[2];
}

void Parameter::getVec4Value(float& x, float& y, float& z, float& w) const {
    if (type_ != osc::ParameterType::VEC4 && type_ != osc::ParameterType::COLOR) {
        throw std::runtime_error("Parameter type mismatch: not a vec4 or color");
    }
    x = data_.vec_values[0];
    y = data_.vec_values[1];
    z = data_.vec_values[2];
    w = data_.vec_values[3];
}

std::string Parameter::toString() const {
    std::stringstream ss;
    switch (type_) {
        case osc::ParameterType::INT:
            ss << data_.int_value;
            break;
        case osc::ParameterType::FLOAT:
            ss << data_.float_value;
            break;
        case osc::ParameterType::STRING:
            ss << string_value_;
            break;
        case osc::ParameterType::BOOL:
            ss << (data_.bool_value ? "true" : "false");
            break;
        case osc::ParameterType::VEC2:
            ss << data_.vec_values[0] << "," << data_.vec_values[1];
            break;
        case osc::ParameterType::VEC3:
            ss << data_.vec_values[0] << "," << data_.vec_values[1] << "," << data_.vec_values[2];
            break;
        case osc::ParameterType::VEC4:
        case osc::ParameterType::COLOR:
            ss << data_.vec_values[0] << "," << data_.vec_values[1] << "," 
               << data_.vec_values[2] << "," << data_.vec_values[3];
            break;
    }
    return ss.str();
}

void Parameter::fromString(const std::string& str) {
    std::stringstream ss(str);
    std::string token;
    
    switch (type_) {
        case osc::ParameterType::INT:
            data_.int_value = std::stoi(str);
            break;
        case osc::ParameterType::FLOAT:
            data_.float_value = std::stof(str);
            break;
        case osc::ParameterType::STRING:
            string_value_ = str;
            break;
        case osc::ParameterType::BOOL:
            data_.bool_value = (str == "true" || str == "1");
            break;
        case osc::ParameterType::VEC2:
            std::getline(ss, token, ',');
            data_.vec_values[0] = std::stof(token);
            std::getline(ss, token, ',');
            data_.vec_values[1] = std::stof(token);
            break;
        case osc::ParameterType::VEC3:
            std::getline(ss, token, ',');
            data_.vec_values[0] = std::stof(token);
            std::getline(ss, token, ',');
            data_.vec_values[1] = std::stof(token);
            std::getline(ss, token, ',');
            data_.vec_values[2] = std::stof(token);
            break;
        case osc::ParameterType::VEC4:
        case osc::ParameterType::COLOR:
            std::getline(ss, token, ',');
            data_.vec_values[0] = std::stof(token);
            std::getline(ss, token, ',');
            data_.vec_values[1] = std::stof(token);
            std::getline(ss, token, ',');
            data_.vec_values[2] = std::stof(token);
            std::getline(ss, token, ',');
            data_.vec_values[3] = std::stof(token);
            break;
    }
}

// Node implementation
Node::Node(int id, const std::string& name, osc::NodeType type)
    : id_(id), name_(name), type_(type), pos_x_(0.0f), pos_y_(0.0f) {
}

void Node::addParameter(std::shared_ptr<Parameter> param) {
    parameters_[param->getName()] = param;
}

std::shared_ptr<Parameter> Node::getParameter(const std::string& name) {
    auto it = parameters_.find(name);
    return (it != parameters_.end()) ? it->second : nullptr;
}

// Connection implementation
Connection::Connection(int id, int source_node_id, const std::string& source_output,
                      int target_node_id, const std::string& target_input)
    : id_(id), source_node_id_(source_node_id), source_output_(source_output),
      target_node_id_(target_node_id), target_input_(target_input) {
}

// NodeGraph implementation
NodeGraph::NodeGraph() : next_node_id_(1), next_connection_id_(1) {
}

void NodeGraph::addNode(std::shared_ptr<Node> node) {
    nodes_[node->getId()] = node;
    next_node_id_ = std::max(next_node_id_, node->getId() + 1);
}

void NodeGraph::removeNode(int node_id) {
    // Remove all connections involving this node
    auto conn_it = connections_.begin();
    while (conn_it != connections_.end()) {
        if (conn_it->second->getSourceNodeId() == node_id || 
            conn_it->second->getTargetNodeId() == node_id) {
            conn_it = connections_.erase(conn_it);
        } else {
            ++conn_it;
        }
    }
    
    // Remove the node
    nodes_.erase(node_id);
}

std::shared_ptr<Node> NodeGraph::getNode(int node_id) {
    auto it = nodes_.find(node_id);
    return (it != nodes_.end()) ? it->second : nullptr;
}

void NodeGraph::addConnection(std::shared_ptr<Connection> connection) {
    connections_[connection->getId()] = connection;
    next_connection_id_ = std::max(next_connection_id_, connection->getId() + 1);
}

void NodeGraph::removeConnection(int connection_id) {
    connections_.erase(connection_id);
}

std::shared_ptr<Connection> NodeGraph::getConnection(int connection_id) {
    auto it = connections_.find(connection_id);
    return (it != connections_.end()) ? it->second : nullptr;
}

void NodeGraph::clear() {
    nodes_.clear();
    connections_.clear();
    next_node_id_ = 1;
    next_connection_id_ = 1;
}

std::vector<std::shared_ptr<Node>> NodeGraph::getTopologicalOrder() const {
    // Simple topological sort implementation
    // For now, just return nodes in order of their IDs
    // TODO: Implement proper topological sorting based on connections
    std::vector<std::shared_ptr<Node>> result;
    for (const auto& pair : nodes_) {
        result.push_back(pair.second);
    }
    return result;
}

std::string NodeGraph::toJSON() const {
    // TODO: Implement JSON serialization
    return "{}";
}

bool NodeGraph::fromJSON(const std::string& json) {
    // TODO: Implement JSON deserialization
    return false;
}

} // namespace gfx
