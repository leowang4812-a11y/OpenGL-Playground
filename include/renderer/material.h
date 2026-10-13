#pragma once

#include "shader.h"
#include "shader_manager.h"
#include "texture_manager.h"

//
struct Material {
	Shader shader;

	std::vector<Texture> diffuse_maps;
	std::vector<Texture> specular_maps;

	const float shininess = 32.0f;

	void setUniforms() const {
		unsigned int tex_unit_ID = 0;

		auto setMaps =
		[&](const std::vector<Texture>& maps, const std::string& prefix_in_shader) {
			for (int i = 0; i < maps.size(); ++i) {
				glActiveTexture(GL_TEXTURE0 + tex_unit_ID);
				glBindTexture(GL_TEXTURE_2D, maps[i].ID);

				shader.setInt("material." + prefix_in_shader + std::to_string(i + 1), tex_unit_ID);
				++tex_unit_ID;
			}
		};

		setMaps(diffuse_maps,  "texture_diffuse");
		setMaps(specular_maps, "texture_specular");

		shader.setFloat("material.shininess", shininess);

		glActiveTexture(GL_TEXTURE0);
	}

	void bind(const glm::mat4& model_mat) const { 
		shader.use(); 
		shader.setMat4("model", model_mat);
		setUniforms();
	}

	bool isOpaque() const {
		for (const auto& tex : diffuse_maps) {
			if (!TexManager::isOpaque(tex)) {
				return false; // failure of full opacity if one texture is transparent.
			}
		}
		return true;
	}

	static Material getDefaultColourMaterial(glm::vec4 colour) {
		Material material{ ShaderManager::getOrLoad("assets/shaders/basic.vert", "assets/shaders/basic.frag", "assets/shaders/basic.geom"), {}, {} };

		material.diffuse_maps.push_back(TexManager::getOrCreateSolidColour(colour));
		material.specular_maps.push_back(TexManager::getOrCreateSolidColour({ 0.1f, 0.1f, 0.1f, 0.1f }));

		return material;
	}
};
