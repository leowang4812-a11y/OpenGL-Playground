#version 460 core

out vec4 frag_colour;
in vec2 tex_coord;

uniform sampler2D screen_texture;
uniform float time_elapsed;

uniform vec2 texel_size;

const float offset_fac = 1 / 1000.0;

float bayer(vec2 p) {
    int x = int(mod(p.x, 4.0));
    int y = int(mod(p.y, 4.0));

    const int m[16] = {
        0,  8,  2, 10,
        12,  4, 14,  6,
        3, 11,  1,  9,
        15,  7, 13,  5
    };

    return (m[y * 4 + x] + 0.5) / 16.0;
}

void main() {

    vec2 uv = tex_coord;

    // pixelate:
    vec2 px = texel_size;
    vec2 step_size = px * 2.0;
    uv = floor(uv / step_size) * step_size + step_size * 0.5; // the second addition forces sampling to the middle.


    // Chromatic Abberration: main idea is to get length of vec from centre of screen, and then 
    // strengthen the effect based on distance squared;

    vec2 dir = uv - 0.5;
    float dist = length(dir);
    float ca = 0.12 * dist  * dist;

    // sample 3 times;

    vec3 colour;

    colour.r = texture(screen_texture, uv + dir * ca, 0.0).r;
    colour.g = texture(screen_texture, uv, 0.0).g;
    colour.b = texture(screen_texture, uv - dir * ca, 0.0).b;

    // exposure
    colour *= 0.8;
    colour = mix(colour, colour * vec3(0.95, 1.03, 0.9), 0.15);

    //dither: the reason why we use true UVs for bayer is because it will undersample for certain step sizes
    float b = bayer(vec2(tex_coord.x / texel_size.x, tex_coord.y / texel_size.y));
    vec3 q = floor(colour * 15.0 + b * 0.35);
    colour = q / (14.0);

    //viginette
    float vig = smoothstep(0.5, 0.9, dist) * 1.0;
    colour *= clamp(1.0 - vig, 0.0, 1.0);

    //add some grain 

    float n = fract(sin(dot(floor(uv / px), vec2(12.9898, 78.233)) + time_elapsed * 37.0) * 43758.5453);
    colour += (n - 0.5) * (px.x + px.y) * 2.0 * 0.5;

    frag_colour = vec4(colour, 1.0);
}