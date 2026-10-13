#version 460 core


struct Mirror {
    sampler2D tex;

    float min_reflectivity;
    float exponent;
};

out vec4 frag_colour;

in vec2 tex_coord;
in vec3 normal;
in vec3 frag_pos;
in vec4 projected_pos;

uniform Mirror mirror;

uniform vec3 view_pos;

void main() {    
    vec3 view_dir = normalize(view_pos - frag_pos);

    vec3 ndc = projected_pos.xyz / projected_pos.w;
    vec2 uv = ndc.xy * 0.5 + 0.5;

    vec4 tex_colour = texture(mirror.tex, uv);

    vec3 fresnal = tex_colour.rgb * (mirror.min_reflectivity + (1 - mirror.min_reflectivity) * (1 - max(0, dot(view_dir, normal))));



	frag_colour = vec4(fresnal, tex_colour.a) * 1.2;
}

