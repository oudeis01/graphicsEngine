#include "graphics/GraphicsEngine.h"
#include "graphics/ShaderManager.h"
#include "graphics/Pipeline.h"
#include <iostream>
#include <memory>
#include <vector>
#include <filesystem>

void testLygiaModules(GraphicsEngine* engine) {
    std::cout << "\n=== Testing LYGIA Module Integration ===" << std::endl;
    
    // ShaderManager 초기화 (GraphicsEngine에서 가져옴)
    // Note: 실제 구현에서는 GraphicsEngine에서 ShaderManager에 접근하는 방법이 필요함
    ShaderManager shaderManager;
    if (!shaderManager.initialize()) {
        std::cerr << "Failed to initialize ShaderManager" << std::endl;
        return;
    }
    
    // 사용 가능한 LYGIA 모듈들 출력
    auto modules = shaderManager.getAvailableModules();
    std::cout << "Available LYGIA modules (" << modules.size() << "):" << std::endl;
    
    // 첫 20개 모듈만 출력 (너무 많으면 생략)
    int count = 0;
    for (const auto& module : modules) {
        if (count++ >= 20) {
            std::cout << "  ... and " << (modules.size() - 20) << " more modules" << std::endl;
            break;
        }
        std::cout << "  " << module << std::endl;
    }
    
    // 간단한 모듈 로딩 테스트 (컴파일 없이)
    std::cout << "\nTesting LYGIA module loading (file access only)..." << std::endl;
    
    // 주요 LYGIA 모듈들이 존재하는지 확인
    std::vector<std::string> testModules = {
        "generative/random.glsl",
        "generative/snoise.glsl",
        "math/rotate2d.glsl",
        "math/const.glsl"
    };
    
    for (const auto& module : testModules) {
        // 파일 존재 여부만 확인
        std::string modulePath = "external/lygia/" + module;
        if (std::filesystem::exists(modulePath)) {
            std::cout << "✓ " << module << " found" << std::endl;
        } else {
            std::cout << "✗ " << module << " not found" << std::endl;
        }
    }
    
    // 통계 출력
    std::cout << "\nShader Manager Statistics:" << std::endl;
    std::cout << "  Total modules discovered: " << modules.size() << std::endl;
    std::cout << "  Cache hits: " << shaderManager.getCacheHits() << std::endl;
    std::cout << "  Hot reloads: " << shaderManager.getHotReloads() << std::endl;
    
    std::cout << "=== LYGIA Module Test Completed ===" << std::endl;
}

int main() {
    std::cout << "Graphics Engine Test Starting..." << std::endl;
    
    // Graphics Engine 초기화
    auto engine = std::make_unique<GraphicsEngine>();
    if (!engine->initialize(800, 600, "Graphics Engine Test - LYGIA Integration")) {
        std::cerr << "Failed to initialize graphics engine" << std::endl;
        return -1;
    }
    
    // LYGIA 모듈 테스트 실행 (OpenGL 컨텍스트 생성 후)
    testLygiaModules(engine.get());
    
    // 기본 셰이더로 테스트 (LYGIA 모듈들을 사용)
    std::cout << "\nStarting main loop with LYGIA-powered default shader..." << std::endl;
    
    // 메인 루프
    engine->mainLoop();
    
    std::cout << "Graphics Engine Test Completed" << std::endl;
    return 0;
}
