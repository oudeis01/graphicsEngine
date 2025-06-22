#version 410 core
out vec4 FragColor;
in vec2 TexCoords;
uniform float time;

#include "noise.glsl"
#include "voronoi.glsl"

void main() {
    vec2 uv = TexCoords * 2.0 - 1.0;
    float n = noise(uv * 5.0, time);
    float v = voronoi(uv * 3.0, time);
    float result = n * v;
    FragColor = vec4(vec3(result), 1.0);
}