#version 410 core

#include "generative/fbm.glsl"
#include "generative/cnoise.glsl"
#include "math/rotate2d.glsl"
#include "color/blend.glsl"

in vec2 TexCoord;
out vec4 FragColor;

uniform float iTime;
uniform vec2 iResolution;

void main() {
    vec2 uv = TexCoord;
    vec2 st = uv * 3.0;
    
    // Animated rotation
    st = rotate2d(iTime * 0.05) * st;
    
    // Generate fractal pattern using LYGIA FBM
    float fbmValue = fbm(st + iTime * 0.1);
    
    // Add curl noise for more complex patterns
    float curlNoise = cnoise(st * 2.0 + vec2(iTime * 0.3, 0.0));
    
    // Create multiple octaves for terrain-like appearance
    vec3 color1 = vec3(0.1, 0.2, 0.8); // Deep blue
    vec3 color2 = vec3(0.8, 0.6, 0.2); // Sandy yellow
    vec3 color3 = vec3(0.2, 0.8, 0.3); // Green
    
    // Blend colors based on height
    vec3 color = mix(color1, color2, smoothstep(0.2, 0.6, fbmValue));
    color = mix(color, color3, smoothstep(0.6, 0.9, fbmValue));
    
    // Add some atmospheric effects
    float atmosphere = 1.0 - length(uv - 0.5) * 0.8;
    color *= atmosphere;
    
    // Add noise detail
    color += curlNoise * 0.1;
    
    FragColor = vec4(color, 1.0);
}
