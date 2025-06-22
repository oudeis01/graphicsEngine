#pragma once
#include <string>

class Shader;

class PipelineParser {
public:
    explicit PipelineParser(const std::string& filename);
    void reload();
    void update(const Shader& shader);

private:
    std::string filename_;
    void parseFile();
};