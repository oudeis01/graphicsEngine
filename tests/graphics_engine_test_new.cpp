#include "graphics/GraphicsEngine.h"
#include "graphics/Pipeline.h"
#include <iostream>
#include <memory>

int main() {
    std::cout << "Graphics Engine Test Starting..." << std::endl;
    
    // Graphics Engine 초기화
    auto engine = std::make_unique<GraphicsEngine>();
    if (!engine->initialize(800, 600, "Graphics Engine Test")) {
        std::cerr << "Failed to initialize graphics engine" << std::endl;
        return -1;
    }
    
    // pipeline.txt 파일 읽기 및 파이프라인 생성
    auto pipeline = Pipeline::fromDescription(R"(
gen n=noise();
gen v=voronoi();
n=multiply(n,v);
output(n,0);
)");
    
    if (pipeline) {
        engine->setPipeline(pipeline);
        std::cout << "Pipeline loaded successfully" << std::endl;
    } else {
        std::cout << "Using default pipeline" << std::endl;
    }
    
    std::cout << "Starting main loop..." << std::endl;
    
    // 메인 루프
    engine->mainLoop();
    
    std::cout << "Graphics Engine Test Completed" << std::endl;
    return 0;
}
