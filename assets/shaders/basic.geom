#version 460 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

layout (location = 0) in vec3 v_frag_pos[];
layout (location = 1) in vec3 v_normal[];
layout (location = 2) in vec2 v_tex_coord[];

layout (location = 0) out vec3 g_frag_pos;
layout (location = 1) out vec3 g_normal;
layout (location = 2) out vec2 g_tex_coord;


void main() {
    for (int i = 0; i < 3; i++) {
        gl_Position = gl_in[i].gl_Position;
        g_frag_pos  = v_frag_pos[i];
        g_tex_coord = v_tex_coord[i];
        g_normal    = v_normal[i];
        
        EmitVertex();
    }


    EndPrimitive();
}