float voronoi(vec2 uv, float t) {
    float d = 1.0;
    for(int x=-1; x<=1; x++) {
        for(int y=-1; y<=1; y++) {
            vec2 gv = floor(uv) + vec2(x, y);
            vec2 r = vec2(x, y) + fract(sin(dot(gv + t, vec2(12.9898, 78.233))) * 43758.5453) - fract(uv);
            float dist = dot(r, r);
            d = min(d, dist);
        }
    }
    return d;
}