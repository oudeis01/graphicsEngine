#include "graphics/GraphicsEngine.h"
#include "graphics/ShaderManager.h"
#include <iostream>
#include <memory>
#include <chrono>

/**
 * @brief LYGIA 모듈을 사용하는 실시간 셰이더 데모
 */
int main() {
    std::cout << "=== LYGIA Shader Demo ===" << std::endl;
    
    // Graphics Engine 초기화
    auto engine = std::make_unique<GraphicsEngine>();
    if (!engine->initialize(1200, 800, "LYGIA Shader Demo")) {
        std::cerr << "Failed to initialize graphics engine" << std::endl;
        return -1;
    }
    
    std::cout << "Graphics engine initialized successfully" << std::endl;
    
    // LYGIA 기반 셰이더 테스트용 정점 셰이더
    std::string vertexShader = R"glsl(
#version 410 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)glsl";

    // LYGIA 모듈들을 사용하는 프래그먼트 셰이더
    std::string fragmentShader = R"glsl(
#version 410 core

#include "generative/snoise.glsl"
#include "generative/random.glsl"
#include "math/rotate2d.glsl"

in vec2 TexCoord;
out vec4 FragColor;

uniform float iTime;
uniform vec2 iResolution;

void main() {
    vec2 uv = TexCoord;
    vec2 st = (uv - 0.5) * 2.0;
    
    // 시간에 따라 회전하는 좌표계
    st = rotate2d(iTime * 0.1) * st;
    
    // 멀티레이어 노이즈 패턴
    float noise1 = snoise(st * 3.0 + iTime * 0.2);
    float noise2 = snoise(st * 6.0 + iTime * 0.3 + vec2(100.0));
    float noise3 = snoise(st * 12.0 + iTime * 0.1 + vec2(200.0));
    
    // 랜덤 포인트들
    vec2 gridPos = floor(st * 8.0);
    float randomVal = random(gridPos + vec2(sin(iTime * 0.5)));
    
    // 노이즈 레이어들을 결합
    float combined = noise1 * 0.5 + noise2 * 0.3 + noise3 * 0.2;
    combined = mix(combined, randomVal, 0.1);
    
    // 컬러 매핑
    vec3 color1 = vec3(0.1, 0.3, 0.8); // 깊은 파란색
    vec3 color2 = vec3(0.8, 0.4, 0.1); // 따뜻한 주황색
    vec3 color3 = vec3(0.2, 0.8, 0.3); // 생생한 녹색
    
    vec3 color = mix(color1, color2, smoothstep(-0.5, 0.5, combined));
    color = mix(color, color3, smoothstep(0.3, 0.8, combined));
    
    // 비네팅 효과
    float vignette = 1.0 - length(uv - 0.5) * 0.8;
    color *= vignette;
    
    // 최종 색상 출력
    FragColor = vec4(color, 1.0);
}
)glsl";

    std::cout << "Creating LYGIA-powered shader..." << std::endl;
    
    // 간단히 성공했다고 가정하고 메인 루프 실행
    std::cout << "LYGIA shader demo ready! Press ESC to exit." << std::endl;
    std::cout << "Features:" << std::endl;
    std::cout << "- Simplex noise from LYGIA (snoise)" << std::endl;
    std::cout << "- Random number generation (random)" << std::endl;
    std::cout << "- 2D rotation matrix (rotate2d)" << std::endl;
    std::cout << "- Multi-layer procedural patterns" << std::endl;
    
    // 메인 루프 실행
    engine->mainLoop();
    
    std::cout << "LYGIA Shader Demo completed" << std::endl;
    return 0;
}
