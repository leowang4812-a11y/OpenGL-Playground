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

struct DirLight {
	vec3 direction; float _pad0;

	vec3 ambient;   float _pad1;
	vec3 diffuse;   float _pad2;
	vec3 specular;  float _pad3;
};

struct PointLight {
	vec3 pos; float _pad;

	vec3 ambient;  float constant;
	vec3 diffuse;  float linear;
	vec3 specular; float quadratic;
};

struct SpotLight {
	vec3 pos;       float inner_cutoff;
	vec3 direction; float outer_cutoff;

	vec3 ambient;  float constant;
	vec3 diffuse;  float linear;
	vec3 specular; float quadratic;
};

#define MAX_DIR_LIGHTS   8
#define MAX_POINT_LIGHTS 8
#define MAX_SPOTLIGHTS   8

layout (std140, binding = 1) uniform Lights {
	int num_dir_lights;
	int num_point_lights;
	int num_spotlights;
	int _pad;

	DirLight   dir_lights[MAX_DIR_LIGHTS];
	PointLight point_lights[MAX_POINT_LIGHTS];
	SpotLight  spotlights[MAX_SPOTLIGHTS];
};

vec3 calcDirLight(DirLight light, vec3 normal, vec3 view_dir, vec3 tex_diffuse, vec3 tex_specular);
vec3 calcPointLight(PointLight light, vec3 normal, vec3 view_dir, vec3 tex_diffuse, vec3 tex_specular);
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 view_dir, vec3 tex_diffuse, vec3 tex_specular);

out vec4 frag_colour;

layout (location = 0) in vec3 frag_pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coord;

uniform Material material;

uniform vec3 view_pos;

void main() {    
	if (texture(material.texture_diffuse1, tex_coord).a < 0.1f) {
		discard;
	}

	vec3 norm = normalize(normal);

	if (!gl_FrontFacing) {
		norm = -norm; 
	}

	vec3 view_dir = normalize(view_pos - frag_pos);
	
	vec4 tex_diffuse = texture(material.texture_diffuse1, tex_coord);
	vec4 tex_specular = texture(material.texture_specular1, tex_coord);

	vec3 result = vec3(0.0f);
	
	for (int i = 0; i < num_dir_lights; ++i) {
		result += calcDirLight(dir_lights[i], norm, view_dir, tex_diffuse.rgb, tex_specular.rgb);
	}

	for (int i = 0; i < num_point_lights; ++i) {
		result += calcPointLight(point_lights[i], norm, view_dir, tex_diffuse.rgb, tex_specular.rgb);	
	}

	for (int i = 0; i < num_spotlights; ++i) {
		result += calcSpotLight(spotlights[i], norm, view_dir, tex_diffuse.rgb, tex_specular.rgb);	
	}

	frag_colour = vec4(result, tex_diffuse.a);
}

vec3 calcDirLight(DirLight light, vec3 normal, vec3 view_dir, vec3 tex_diffuse, vec3 tex_specular) {
	vec3 light_dir = normalize(-light.direction);
	vec3 reflect_dir = reflect(-light_dir, normal);

	float diffuse_strength  = max(dot(normal, light_dir), 0.0);
	float specular_strength = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);

	vec3 ambient  = light.ambient * tex_diffuse;
	vec3 diffuse  = light.diffuse * diffuse_strength * tex_diffuse;
	vec3 specular = light.specular * specular_strength * tex_specular;

	return (ambient + diffuse + specular);
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 view_dir, vec3 tex_diffuse, vec3 tex_specular) {
	vec3 light_dir = normalize(light.pos - frag_pos);
	vec3 reflect_dir = reflect(-light_dir, normal);

	float diffuse_strength = max(dot(normal, light_dir), 0.0);
	float specular_strength = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);

	float distance = length(light.pos - frag_pos); // optimise by normalising inside function using distance
	float attenuation = 1 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

	vec3 diffuse  = light.diffuse * diffuse_strength * tex_diffuse;
	vec3 specular = light.specular * specular_strength * tex_specular;

	return (diffuse + specular) * attenuation;
}

vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 view_dir, vec3 tex_diffuse, vec3 tex_specular) {
	float distance = length(light.pos - frag_pos); 

	vec3 frag_to_source = (light.pos - frag_pos) / distance; 

	float cos_theta = dot(frag_to_source, normalize(-light.direction));

	if (cos_theta > light.outer_cutoff) {
		float attenuation = 1 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

		float cos_difference = light.inner_cutoff - light.outer_cutoff;
		float spotlight_intensity = clamp((cos_theta - light.outer_cutoff) / cos_difference, 0.0, 1.0);

		vec3 frag_to_camera = normalize(-frag_pos);
		vec3 reflect_vec = reflect(-frag_to_source, normal); 

		float diffuse_strength = max(dot(normal, frag_to_source), 0.0f);
		float specular_strength = pow(max(dot(frag_to_camera, reflect_vec), 0.0f), material.shininess);

		vec3 diffuse  = light.diffuse * diffuse_strength * tex_diffuse;
		vec3 specular = light.specular * specular_strength * tex_specular;
		
		return (diffuse + specular) * spotlight_intensity * attenuation;
	}

	return vec3(0.0f);
}