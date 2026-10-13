#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTex_coord;

out vec3 frag_pos;
out vec2 tex_coord;
out vec3 normal;
out vec4 projected_pos;

uniform mat4 model; 
uniform mat4 mirror_view_proj;
layout (std140, binding = 0) uniform Matrices {
	mat4 view;
	mat4 proj;
};



void main(){
    mat4 model_view = view * model;
    gl_Position = proj * model_view * vec4(aPos, 1.0);

    // pass texture coordinates, normal, and fragment position to the fragment shader
    frag_pos  = vec3(model * vec4(aPos, 1.0));
    tex_coord = aTex_coord;
    normal    = mat3(transpose(inverse(model))) * aNormal; // transpose inverse correctly maintains normal to surface.
    projected_pos = mirror_view_proj * model * vec4(aPos, 1.0);
}