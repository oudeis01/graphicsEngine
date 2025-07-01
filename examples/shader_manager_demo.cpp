#include "graphics/GraphicsEngine.h"
#include "graphics/ShaderManager.h"
#include "graphics/NodeEditor.h"
#include <iostream>

int main() {
    std::cout << "=== Graphics Engine & Node Editor Integration Demo ===" << std::endl;
    
    // Shader Manager 초기화 및 테스트
    ShaderManager shaderManager;
    
    // LYGIA 라이브러리 로드
    std::string lygiaPath = "../external/lygia";  
    if (shaderManager.initializeLygia(lygiaPath)) {
        std::cout << "✅ LYGIA initialized successfully" << std::endl;
        
        auto modules = shaderManager.getAvailableModules();
        std::cout << "📦 Loaded " << modules.size() << " LYGIA modules" << std::endl;
        
        // 몇 가지 흥미로운 모듈 출력
        std::cout << "\n🔍 Sample modules:" << std::endl;
        int count = 0;
        for (const auto& module : modules) {
            if (count >= 10) break;
            if (module.find("color") != std::string::npos || 
                module.find("noise") != std::string::npos ||
                module.find("math") != std::string::npos) {
                std::cout << "  • " << module << std::endl;
                count++;
            }
        }
    } else {
        std::cout << "⚠️  Warning: LYGIA initialization failed" << std::endl;
    }
    
    // 커스텀 셰이더 모듈 등록
    shaderManager.registerShaderModule("animated_color", R"(
        vec3 animated_color(vec2 uv, float time) {
            float r = 0.5 + 0.5 * sin(time + uv.x * 10.0);
            float g = 0.5 + 0.5 * sin(time + uv.y * 8.0 + 2.0);
            float b = 0.5 + 0.5 * sin(time + (uv.x + uv.y) * 6.0 + 4.0);
            return vec3(r, g, b);
        }
    )");
    
    std::cout << "✅ Registered custom shader module 'animated_color'" << std::endl;
    
    // 셰이더 조합 테스트
    std::vector<std::string> modules = {"animated_color"};
    auto composition = shaderManager.composeShader(modules);
    
    std::cout << "\n🧩 Shader composition test:" << std::endl;
    std::cout << "  • Vertex shader: " << composition.vertexSource.size() << " characters" << std::endl;
    std::cout << "  • Fragment shader: " << composition.fragmentSource.size() << " characters" << std::endl;
    std::cout << "  • Included modules: " << composition.includedModules.size() << std::endl;
    
    // 의존성 그래프 출력
    std::cout << "\n📊 Dependency graph:" << std::endl;
    shaderManager.printDependencyGraph();
    
    // ShaderManager의 디버그 정보 출력
    std::cout << "\n📋 Debug info:" << std::endl;
    std::cout << shaderManager.generateDebugInfo() << std::endl;
    
    std::cout << "🎯 Shader Manager test completed successfully!" << std::endl;
    std::cout << "\nNow starting Graphics Engine with Node Editor..." << std::endl;
    
    // Graphics Engine 초기화 및 실행 (Node Editor 포함)
    try {
        GraphicsEngine engine;
        if (engine.initialize(800, 600, "Graphics Engine + Node Editor Demo")) {
            std::cout << "✅ Graphics Engine initialized" << std::endl;
            std::cout << "🚀 Starting render loop..." << std::endl;
            std::cout << "💡 Press F1 to toggle Node Editor window" << std::endl;
            std::cout << "💡 Close main window to exit" << std::endl;
            
            // 메인 렌더 루프 실행
            engine.run();
            
            std::cout << "🏁 Render loop completed" << std::endl;
        } else {
            std::cerr << "❌ Failed to initialize Graphics Engine" << std::endl;
            return -1;
        }
    } catch (const std::exception& e) {
        std::cerr << "❌ Graphics Engine error: " << e.what() << std::endl;
        return -1;
    }
    
    std::cout << "🎉 Demo completed successfully!" << std::endl;
    return 0;
}
