#version 410 core

out vec2 TexCoords;

void main() {
    // Generate a fullscreen triangle using gl_VertexID
    // This creates vertices at (-1,-1), (3,-1), (-1,3) covering the entire screen
    vec2 vertices[3] = vec2[3](
        vec2(-1.0, -1.0),
        vec2( 3.0, -1.0),
        vec2(-1.0,  3.0)
    );
    
    vec2 pos = vertices[gl_VertexID];
    TexCoords = (pos + 1.0) / 2.0;
    gl_Position = vec4(pos, 0.0, 1.0);
}