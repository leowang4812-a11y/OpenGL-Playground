#pragma once

#include "material.h"
#include "mesh.h"
#include "scene.h"
#include "camera.h"
#include "texture_manager.h"
#include "object.h"


class Renderer {
public:
	Renderer() = default;

	void testRender(const ViewBasis& basis) {
		// test mesh
		Texture tex = TexManager::getOrLoad("container.jpg", "assets/textures/");
		Material mat(ShaderManager::getOrLoad("assets/shaders/basic.vert", "assets/shaders/test.frag"), { tex }, {});

		// Triangle vertices
		std::vector<Vertex> m_vertices = {
			// position                  // normal            // uv
			{ glm::vec3( 0.0f,  0.5f, 0.0f), glm::vec3(0,0,1), glm::vec2(0.5f, 1.0f) },
			{ glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0,0,1), glm::vec2(0.0f, 0.0f) },
			{ glm::vec3( 0.5f, -0.5f, 0.0f), glm::vec3(0,0,1), glm::vec2(1.0f, 0.0f) }
		};
		// One triangle
		std::vector<unsigned int> m_indices = {
			0, 1, 2
		};

		Mesh m(m_vertices, m_indices, mat);
		Transform transform{};

		Shader shader = m.m_material.shader;
		shader.use();
		shader.setMat4("model", glm::mat4(1.0f));
		shader.setVec3("view_pos", basis.pos);
		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_2D, tex.ID);
		shader.setInt("material.texture_diffuse1", 0);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		m.draw();

	}

	void render(const Scene& scene, const ViewBasis& basis, float time_elapsed) {
		scene.lUBO.setData(scene.testLights());

		const Transform& scene_transform = scene.transform;
		for (const auto& object : scene.objects) {
			renderObject(object, scene_transform, basis);
			// (TODO) eventually separate into a component based approach to allow for more flexible entities and to allow for more complex behaviour such as animation.
		}

		for (const auto& terrain : scene.terrains) {
			const TerrainMesh& terrain_mesh = *terrain.terrain_mesh;

			terrain_mesh.mesh.m_material.bind(terrain.transform.computeModelMatrix());	
			terrain_mesh.mesh.m_material.shader.setVec3("view_pos", basis.pos);

			terrain_mesh.draw();
		}
	}

	void renderCamera(const Camera& camera, const ViewBasis& basis) {
		glm::mat4 rotate_90 = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3{0.0f, 1.0f, 0.0f});
		for (auto& model_instance : camera.object.instances) {
			glm::mat4 model_transform = model_instance.transform.computeModelMatrix();
			for (auto& mesh : model_instance.meshes) {

				mesh->m_material.bind(camera.basis.getModelMatrix() * rotate_90 * model_transform);	
				mesh->m_material.shader.setVec3("view_pos", basis.pos);

				mesh->draw();
			}
		}
	}


private:
	void renderObject(const ObjectInstance& object, const Transform& parent_transform, const ViewBasis& basis) {
		Transform world_transform = Transform::combine(parent_transform, object.t);
		for (auto& model_instance : object.instances) {
			Transform model_transform = Transform::combine(world_transform, model_instance.transform);
			for (auto& mesh : model_instance.meshes) {
				mesh->m_material.bind(model_transform.computeModelMatrix());	
				mesh->m_material.shader.setVec3("view_pos", basis.pos);

				mesh->draw();
			}
		}
	}
};
