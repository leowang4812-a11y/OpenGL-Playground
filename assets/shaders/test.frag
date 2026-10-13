#version 460 core
struct Material {
    sampler2D texture_diffuse1;
	sampler2D texture_diffuse2;
	sampler2D texture_diffuse3;
	sampler2D texture_diffuse4;
	sampler2D texture_specular1;
	sampler2D texture_specular2;
	sampler2D texture_specular3;
	sampler2D texture_specular4;

	float shininess;
};

uniform Material material;

out vec4 FragColor;

in vec3 frag_pos;
in vec2 tex_coord;
in vec3 normal;

void main()
{
    FragColor = texture(material.texture_diffuse1, tex_coord);
}