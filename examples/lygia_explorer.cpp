#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

/**
 * @brief 간단한 LYGIA 모듈 탐색 및 테스트 유틸리티
 */
int main() {
    std::cout << "=== LYGIA Module Explorer ===" << std::endl;
    
    std::string lygiaPath = "../external/lygia";
    
    if (!std::filesystem::exists(lygiaPath)) {
        std::cerr << "Error: LYGIA path not found: " << lygiaPath << std::endl;
        return 1;
    }
    
    std::cout << "Scanning LYGIA modules in: " << lygiaPath << std::endl;
    
    std::vector<std::string> modules;
    
    // 재귀적으로 .glsl 파일들 찾기
    for (const auto& entry : std::filesystem::recursive_directory_iterator(lygiaPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".glsl") {
            std::string relativePath = std::filesystem::relative(entry.path(), lygiaPath).string();
            modules.push_back(relativePath);
        }
    }
    
    std::cout << "Found " << modules.size() << " GLSL modules:" << std::endl;
    
    // 카테고리별로 정리해서 출력
    std::vector<std::string> categories = {"math/", "generative/", "color/", "filter/", "lighting/", "geometry/"};
    
    for (const auto& category : categories) {
        std::cout << "\n" << category << ":" << std::endl;
        int count = 0;
        for (const auto& module : modules) {
            if (module.starts_with(category)) {
                std::cout << "  " << module << std::endl;
                count++;
                if (count >= 10) {
                    int remaining = 0;
                    for (const auto& remaining_module : modules) {
                        if (remaining_module.starts_with(category)) remaining++;
                    }
                    if (remaining > 10) {
                        std::cout << "  ... and " << (remaining - 10) << " more " << category << " modules" << std::endl;
                    }
                    break;
                }
            }
        }
    }
    
    // 몇 가지 핵심 모듈들의 내용 미리보기
    std::cout << "\n=== Sample Module Contents ===" << std::endl;
    
    std::vector<std::string> sampleModules = {
        "math/rotate2d.glsl",
        "generative/random.glsl",
        "generative/snoise.glsl"
    };
    
    for (const auto& module : sampleModules) {
        std::string fullPath = lygiaPath + "/" + module;
        std::cout << "\n--- " << module << " ---" << std::endl;
        
        std::ifstream file(fullPath);
        if (file.is_open()) {
            std::string line;
            int lineCount = 0;
            while (std::getline(file, line) && lineCount < 20) {
                std::cout << line << std::endl;
                lineCount++;
            }
            if (lineCount >= 20) {
                std::cout << "... (truncated)" << std::endl;
            }
        } else {
            std::cout << "Could not open file: " << fullPath << std::endl;
        }
    }
    
    std::cout << "\n=== LYGIA Module Explorer Complete ===" << std::endl;
    return 0;
}
