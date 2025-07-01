#version 410 core

#include "generative/random.glsl"
#include "generative/snoise.glsl"
#include "math/rotate2d.glsl"

in vec2 TexCoord;
out vec4 FragColor;

uniform float iTime;
uniform vec2 iResolution;

void main() {
    vec2 uv = TexCoord;
    vec2 st = uv * 2.0 - 1.0;
    
    // Rotate UV coordinates
    st = rotate2d(iTime * 0.2) * st;
    
    // Generate noise pattern using LYGIA
    float noise = snoise(st * 3.0 + iTime * 0.5);
    float randomValue = random(st + iTime);
    
    // Create animated pattern
    vec3 color = vec3(
        0.5 + 0.5 * noise,
        0.5 + 0.3 * sin(iTime + randomValue * 6.28318),
        0.7 + 0.3 * cos(iTime * 0.7 + noise * 3.14159)
    );
    
    // Add some procedural detail
    float detail = snoise(st * 8.0 + iTime * 0.3) * 0.1;
    color += detail;
    
    FragColor = vec4(color, 1.0);
}
