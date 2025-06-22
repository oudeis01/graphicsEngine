#include "PipelineParser.h"
#include "Shader.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <random>

PipelineParser::PipelineParser(const std::string& filename) : filename_(filename) {
    parseFile();
}

void PipelineParser::reload() {
    parseFile();
}

void PipelineParser::update(const Shader& shader) {
    // Apply parsed pipeline operations to shader uniforms
    // For now, just set some example uniforms based on the operations
    static float time = 0.0f;
    time += 0.016f; // roughly 60fps
    
    shader.setFloat("time", time);
    shader.setFloat("noiseScale", 1.0f);
    shader.setFloat("voronoiScale", 1.0f);
}

void PipelineParser::parseFile() {
    std::ifstream file(filename_);
    if (!file.is_open()) {
        std::cerr << "Failed to open pipeline file: " << filename_ << std::endl;
        return;
    }
    
    std::string line;
    std::map<std::string, std::string> variables;
    
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;
        
        // Parse variable assignments and function calls
        std::istringstream iss(line);
        std::string token;
        
        if (line.find('=') != std::string::npos) {
            // Variable assignment
            size_t eqPos = line.find('=');
            std::string varName = line.substr(0, eqPos);
            std::string expression = line.substr(eqPos + 1);
            
            // Remove whitespace
            varName.erase(0, varName.find_first_not_of(" \t"));
            varName.erase(varName.find_last_not_of(" \t") + 1);
            expression.erase(0, expression.find_first_not_of(" \t"));
            expression.erase(expression.find_last_not_of(" \t") + 1);
            
            variables[varName] = expression;
            
            std::cout << "Parsed: " << varName << " = " << expression << std::endl;
        }
    }
    
    file.close();
}