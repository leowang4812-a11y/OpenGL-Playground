#pragma once

#include "material.h"

glm::vec3 getVertexWeightedCentroid(const std::vector<Vertex>& vertices) {
	glm::vec3 centroid(0.0f);
	for (const auto& vertex : vertices) {
		centroid += vertex.pos;
	}

	if (vertices.size() > 0) centroid /= vertices.size();

	return centroid;
}


struct Mesh {
	Material m_material;
    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;
	
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, const Material& material)
		: m_vertices{ vertices }
		, m_indices{ indices }
		, m_material{ material } {

		setupMesh();
	}
	
	void draw() const {
		glBindVertexArray(m_VAO);
		glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0); // unbind VAO
	}

	// compromise in encapsulation by allowing mesh to intialise itself in opengl but allowing draw to occur in the renderer
	void setupMesh() { 
		glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_VBO);
		glGenBuffers(1, &m_EBO);

		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), &m_vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0], GL_STATIC_DRAW);

		//configure vector attributes
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords));
		glEnableVertexAttribArray(2);
	}

	void shiftVertices(glm::vec3 offset) {
		for (auto& vertex : m_vertices) {
		vertex.pos -= offset;
		//vertex.pos -= glm::vec3{ Random::getNormalDist(0.0f, 10.0f), Random::getNormalDist(0.0f, 10.0f), Random::getNormalDist(0.0f, 10.0f) };
		}
	}

	GLuint m_VAO = 0, m_VBO = 0, m_EBO = 0;
};

struct TerrainMesh {
	Mesh mesh;
	int width, height;

	void draw() const {
		int num_strips = height - 1;
		int vertices_per_strip = width * 2;

		for (int strip = 0; strip < num_strips; ++strip) {

			glBindVertexArray(mesh.m_VAO);
			glDrawElements(GL_TRIANGLE_STRIP, vertices_per_strip, GL_UNSIGNED_INT, (void*)(strip * vertices_per_strip * sizeof(unsigned int)));
			glBindVertexArray(0); // unbind VAO
		}
	}

	TerrainMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, const Material& material, int width, int height) 
		: mesh(vertices, indices, material), width(width), height(height) {
	}

};

Mesh screenQuad() {
	std::vector<Vertex> vertices{
		{ { -1.0f,  1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
		{ { -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
		{ {  1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },

		{ { -1.0f,  1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
		{ { 1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },
		{ { 1.0f,  1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },										
	};

	std::vector<unsigned int> indices {
		0, 1, 2, 3, 4, 5
	};

    return Mesh(vertices, indices, Material());
}

TerrainMesh terrainGen(const std::string& heightmap_path="assets/heightmaps/iceland_heightmap.png") {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	Material mat = Material::getDefaultColourMaterial(glm::vec4{ 0.5f, 0.5f, 0.5f, 1.0f });

	std::cout << "Generating terrain mesh from heightmap: " << heightmap_path << '\n';
	auto [data, width, height, nr_channels] = getImgData(heightmap_path);

	float y_shift = 100.0f;
	float y_scale = 200.0f / 256.0f;
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {

			auto getHeight = [&] (int x, int z) -> float {
				if (x < 0 || x >= width || z < 0 || z >= height) return 0.0f;
				unsigned char* texel = data + (x + width * z) * nr_channels;

				return texel[0] * y_scale - y_shift;
			};


			glm::vec3 pos = {
				-width /2.0f + j,
				getHeight(j, i),
				-height /2.0f + i
			};

			constexpr glm::vec3 DEFAULT_WORLD_UP(0.0f, 1.0f, 0.0f);
			// create normal:

			float x_height_diff = pos.y - getHeight(j + 1, i);
			float z_height_diff = pos.y - getHeight(j, i + 1);

			glm::vec3 vecx = glm::normalize(glm::vec3{ 1.0f, x_height_diff, 0.0f });
			glm::vec3 vecz = glm::normalize(glm::vec3{ 0.0f, z_height_diff, 1.0f });
			glm::vec3 normal = glm::normalize(glm::cross(vecz, vecx));
			
			if (glm::dot(normal, DEFAULT_WORLD_UP) < 0.0f) {
				normal = -normal;
			}

			// will sample from 1x1 solid texture in shader, so tex coords don't matter
			vertices.push_back(Vertex{ pos, normal, glm::vec2{ 0.1f, 0.1f }});
		}
	}

	for (int i = 0; i < height - 1; ++i) {
		for (int j = 0; j < width; ++j) {
			for (int k = 0; k < 2; ++k) {
				indices.push_back(width * (i + k) + j);
			} 
		}
	}

	freeImgData(data);

	return TerrainMesh(vertices, indices, mat, width, height);
}