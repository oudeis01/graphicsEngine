#pragma once

#include <string>
#include <map>
#include <memory>
#include <vector>
#include "../osc/OSCMessages.h"

namespace gfx {

// Forward declarations
class Node;
class Connection;

// Parameter value holder
class Parameter {
public:
    Parameter(const std::string& name, osc::ParameterType type);
    
    // Getters
    const std::string& getName() const { return name_; }
    osc::ParameterType getType() const { return type_; }
    
    // Type-specific setters
    void setValue(int value);
    void setValue(float value);
    void setValue(const std::string& value);
    void setValue(bool value);
    void setValue(float x, float y); // vec2
    void setValue(float x, float y, float z); // vec3
    void setValue(float x, float y, float z, float w); // vec4
    
    // Type-specific getters
    int getIntValue() const;
    float getFloatValue() const;
    const std::string& getStringValue() const;
    bool getBoolValue() const;
    void getVec2Value(float& x, float& y) const;
    void getVec3Value(float& x, float& y, float& z) const;
    void getVec4Value(float& x, float& y, float& z, float& w) const;
    
    // Generic string representation
    std::string toString() const;
    void fromString(const std::string& str);
    
private:
    std::string name_;
    osc::ParameterType type_;
    
    // Union for storing different types
    union {
        int int_value;
        float float_value;
        bool bool_value;
        float vec_values[4]; // For vec2, vec3, vec4
    } data_;
    
    std::string string_value_; // Separate storage for strings
};

// Node base class
class Node {
public:
    Node(int id, const std::string& name, osc::NodeType type);
    virtual ~Node() = default;
    
    // Basic info
    int getId() const { return id_; }
    const std::string& getName() const { return name_; }
    osc::NodeType getType() const { return type_; }
    
    // Parameters
    void addParameter(std::shared_ptr<Parameter> param);
    std::shared_ptr<Parameter> getParameter(const std::string& name);
    const std::map<std::string, std::shared_ptr<Parameter>>& getParameters() const { return parameters_; }
    
    // Position (for visual representation)
    void setPosition(float x, float y) { pos_x_ = x; pos_y_ = y; }
    void getPosition(float& x, float& y) const { x = pos_x_; y = pos_y_; }
    
    // Processing (to be implemented by derived classes)
    virtual void process() = 0;
    virtual void initialize() {}
    virtual void cleanup() {}
    
protected:
    int id_;
    std::string name_;
    osc::NodeType type_;
    std::map<std::string, std::shared_ptr<Parameter>> parameters_;
    float pos_x_, pos_y_;
};

// Connection between nodes
class Connection {
public:
    Connection(int id, int source_node_id, const std::string& source_output,
               int target_node_id, const std::string& target_input);
    
    int getId() const { return id_; }
    int getSourceNodeId() const { return source_node_id_; }
    const std::string& getSourceOutput() const { return source_output_; }
    int getTargetNodeId() const { return target_node_id_; }
    const std::string& getTargetInput() const { return target_input_; }
    
private:
    int id_;
    int source_node_id_;
    std::string source_output_;
    int target_node_id_;
    std::string target_input_;
};

// Graph that holds nodes and connections
class NodeGraph {
public:
    NodeGraph();
    ~NodeGraph() = default;
    
    // Node management
    void addNode(std::shared_ptr<Node> node);
    void removeNode(int node_id);
    std::shared_ptr<Node> getNode(int node_id);
    const std::map<int, std::shared_ptr<Node>>& getNodes() const { return nodes_; }
    
    // Connection management
    void addConnection(std::shared_ptr<Connection> connection);
    void removeConnection(int connection_id);
    std::shared_ptr<Connection> getConnection(int connection_id);
    const std::map<int, std::shared_ptr<Connection>>& getConnections() const { return connections_; }
    
    // Graph operations
    void clear();
    std::vector<std::shared_ptr<Node>> getTopologicalOrder() const;
    
    // Serialization
    std::string toJSON() const;
    bool fromJSON(const std::string& json);
    
private:
    std::map<int, std::shared_ptr<Node>> nodes_;
    std::map<int, std::shared_ptr<Connection>> connections_;
    int next_node_id_;
    int next_connection_id_;
};

} // namespace gfx
