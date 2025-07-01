#version 410 core

#include "generative/voronoi.glsl"
#include "generative/snoise.glsl"
#include "math/rotate2d.glsl"

in vec2 TexCoord;
out vec4 FragColor;

uniform float iTime;
uniform vec2 iResolution;

void main() {
    vec2 uv = TexCoord;
    vec2 st = uv * 5.0;
    
    // Animate the pattern
    st = rotate2d(iTime * 0.1) * st;
    st += snoise(st * 0.5 + iTime * 0.2) * 0.5;
    
    // Generate Voronoi pattern using LYGIA
    vec3 voronoiResult = voronoi(st);
    float dist = voronoiResult.x; // Distance to closest point
    
    // Add some noise variation
    float noise = snoise(st * 2.0 + iTime * 0.3);
    
    // Create color based on Voronoi cells
    vec3 color = vec3(
        0.2 + 0.8 * smoothstep(0.0, 0.1, dist),
        0.5 + 0.5 * sin(voronoiResult.y * 6.28318 + iTime),
        0.7 + 0.3 * noise
    );
    
    // Add cell borders
    float border = smoothstep(0.02, 0.05, dist);
    color *= border;
    
    FragColor = vec4(color, 1.0);
}
