#include "PipelineModule.h"
#include <iostream>

// Static member initialization
std::unordered_map<std::string, std::function<std::unique_ptr<PipelineModule>()>> ModuleFactory::creators_;

PipelineModule::PipelineModule(const std::string& name, ModuleType type)
    : name_(name), type_(type) {
}

void ModuleFactory::registerModule(const std::string& name, 
                                  std::function<std::unique_ptr<PipelineModule>()> creator) {
    creators_[name] = creator;
    std::cout << "Registered module: " << name << std::endl;
}

std::unique_ptr<PipelineModule> ModuleFactory::createModule(const std::string& name) {
    auto it = creators_.find(name);
    if (it != creators_.end()) {
        return it->second();
    }
    std::cerr << "Unknown module: " << name << std::endl;
    return nullptr;
}

std::vector<std::string> ModuleFactory::getAvailableModules() {
    std::vector<std::string> modules;
    for (const auto& [name, creator] : creators_) {
        modules.push_back(name);
    }
    return modules;
}
