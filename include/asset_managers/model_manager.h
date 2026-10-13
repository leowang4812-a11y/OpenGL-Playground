#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "material.h"
#include "mesh.h"

struct MeshGroup {
	std::vector<Mesh> meshes;
};

struct ModelInstance {
	std::vector<const Mesh*> meshes;
	Transform transform;
};

struct TerrainInstance {
	TerrainMesh* terrain_mesh;
	Transform transform;
};

std::vector<ModelInstance> processNode(aiNode* node, const std::string& tex_dir, std::shared_ptr<MeshGroup>& mg,
	std::vector<ModelInstance>& instances_so_far, const Transform& parent_transform);
Mesh processMesh(aiMesh* mesh, const aiScene* scene, const std::string& tex_dir);
std::vector<Texture> loadMaterial(aiMaterial* material, aiTextureType type, const std::string& tex_dir);
std::shared_ptr<MeshGroup> adaptAssimpScene(const aiScene* scene, const std::string& tex_dir);

// (TODO) each node in assimp has a shared transform. This node may have multiple meshes. Thus, it is desirable to group these meshes into one model instance.

// Two ways to get model instances: recieve them directly via assimp, or get the expensive geometry meshes directly and make your own models.

class ModelManager {
public:
	static std::vector<ModelInstance> genFromMesh(const Mesh& mesh) {
		ModelInstance instance;
		instance.meshes.push_back(&mesh);
		return { instance };
	}

	static std::vector<ModelInstance> loadInstances(const std::string& path) {
    	Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

		if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
			std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << '\n';
			return {};
		}
		std::string tex_dir = path.substr(0, path.find_last_of('/'));

		std::shared_ptr<MeshGroup> mg;

        auto it = cache.find(path);
        if (it != cache.end()) {
			mg = it->second;
		}
		else {
			cache[path] = adaptAssimpScene(scene, tex_dir);
			mg = cache[path];
		}

		std::vector<ModelInstance> models;
		Transform accum{};
        return processNode(scene->mRootNode, tex_dir, cache[path], models, accum);
    }
private:
    static inline AssetMap<MeshGroup> cache;
};

std::shared_ptr<MeshGroup> adaptAssimpScene(const aiScene* scene, const std::string& tex_dir) {
	MeshGroup mg;
	for (int i = 0; i < scene->mNumMeshes; ++i) {
		aiMesh* mesh = scene->mMeshes[i];

		mg.meshes.push_back(processMesh(mesh, scene, tex_dir));
	}
	return std::make_shared<MeshGroup>(mg);
}

// recursively processes all nodes
std::vector<ModelInstance> processNode(aiNode* node, const std::string& tex_dir, std::shared_ptr<MeshGroup>& mg,
	std::vector<ModelInstance>& instances_so_far, const Transform& parent_transform) {

	aiMatrix4x4 t = node->mTransformation;

	aiVector3D scale, pos; aiQuaternion quat;
	t.Decompose(scale, quat, pos);

	Transform local_t;

	local_t.scale.x = scale.x;
	local_t.scale.y = scale.y;
	local_t.scale.z = scale.z;

	local_t.quat.x = quat.x;
	local_t.quat.y = quat.y;
	local_t.quat.z = quat.z;
	local_t.quat.w = quat.w;

	local_t.pos.x = pos.x;
	local_t.pos.y = pos.y;
	local_t.pos.z = pos.z;

	Transform world_transform = Transform::combine(parent_transform, local_t);

	ModelInstance instance;
	instance.transform = world_transform;
	for (int i = 0; i < node->mNumMeshes; ++i) {
		instance.meshes.push_back(&mg->meshes[node->mMeshes[i]]);
	}
	instances_so_far.push_back(instance);

	for (int i = 0; i < node->mNumChildren; ++i) {
		processNode(node->mChildren[i], tex_dir, mg, instances_so_far, world_transform);
	}

	return instances_so_far;
}

Mesh processMesh(aiMesh* mesh, const aiScene* scene, const std::string& tex_dir) {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	Material mat{};
	mat.shader = ShaderManager::getOrLoad("assets/shaders/basic.vert", "assets/shaders/basic.frag", "assets/shaders/basic.geom");
	for (int i = 0; i < mesh->mNumVertices; ++i) {
		Vertex vertex;

		glm::vec3 temp_vec;
		temp_vec.x = mesh->mVertices[i].x;
		temp_vec.y = mesh->mVertices[i].y;
		temp_vec.z = mesh->mVertices[i].z;
		vertex.pos = temp_vec;

		temp_vec.x = mesh->mNormals[i].x;
		temp_vec.y = mesh->mNormals[i].y;
		temp_vec.z = mesh->mNormals[i].z;
		vertex.normal = temp_vec;

		if (mesh->mTextureCoords[0]) {
			glm::vec2 texture_coords;
			texture_coords.x = mesh->mTextureCoords[0][i].x;
			texture_coords.y = mesh->mTextureCoords[0][i].y;
			vertex.tex_coords = texture_coords;
		}
		else {
			vertex.tex_coords = glm::vec2{ 0.0f, 0.0f };
		}

		vertices.emplace_back(vertex);
	}

	for (int i = 0; i < mesh->mNumFaces; ++i) {
		aiFace face = mesh->mFaces[i];

		for (int j = 0; j < face.mNumIndices; ++j) {
			indices.emplace_back(face.mIndices[j]);
		}
	}

	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

	if (mesh->mMaterialIndex < 0) { // if smaller than 0, then the mesh doesn't have a material 
		mat.diffuse_maps.emplace_back(TexManager::getDefaultWhite());

	}
	else if (material->GetTextureCount(aiTextureType_DIFFUSE) == 0) { // if it has a material but no diffuse maps -> adds solid colour maps
		aiColor3D diffuse { 0.0f };
		aiColor3D specular{ 0.0f };
		float shininess   { 0.0f };

		material->Get(AI_MATKEY_COLOR_DIFFUSE,  diffuse);
		material->Get(AI_MATKEY_COLOR_SPECULAR, specular);
		material->Get(AI_MATKEY_SHININESS,      shininess);

		mat.diffuse_maps.emplace_back(TexManager::getOrCreateSolidColour({ diffuse[0], diffuse[1], diffuse[2], 1.0f }));
		mat.specular_maps.emplace_back(TexManager::getOrCreateSolidColour({ specular[0], specular[1], specular[2], 1.0f }));

	}
	else if (mesh->mMaterialIndex >= 0) { // unnecessary but for documentation purposes

		std::vector<Texture> diffuse_maps = loadMaterial(material, aiTextureType_DIFFUSE, tex_dir);
		mat.diffuse_maps.insert(mat.diffuse_maps.end(), diffuse_maps.begin(), diffuse_maps.end());

		std::vector<Texture> specular_maps = loadMaterial(material, aiTextureType_SPECULAR, tex_dir);
		mat.specular_maps.insert(mat.specular_maps.end(), specular_maps.begin(), specular_maps.end());
		std::cout << diffuse_maps.size() << '\n';
		// debug to check if maps loaded
		aiString diffusePath;
		if (material->GetTexture(aiTextureType_DIFFUSE, 0, &diffusePath) == AI_SUCCESS) {
			std::cout << "Diffuse texture path: " << diffusePath.C_Str() << std::endl;
			std::cout << "Diffuse texture path: " << tex_dir << std::endl;
		}

		// Load Specular Texture (map_Ks)
		aiString specularPath;
		if (material->GetTexture(aiTextureType_SPECULAR, 0, &specularPath) == AI_SUCCESS) {
			std::cout << "Specular texture path: " << specularPath.C_Str() << std::endl;
			std::cout << "Specular texture path: " << tex_dir << std::endl;
		}

		// Load Normal Map (map_Bump or map_normal)
		aiString normalPath;
		if (material->GetTexture(aiTextureType_NORMALS, 0, &normalPath) == AI_SUCCESS) {
			std::cout << "Normal map path: " << normalPath.C_Str() << std::endl;
		}
	}

	return Mesh{ vertices, indices, mat };
}

std::vector<Texture> loadMaterial(aiMaterial* material, aiTextureType type, const std::string& tex_dir) {
	std::vector<Texture> textures;

	for (int i = 0; i < material->GetTextureCount(type); ++i) {
		aiString texture_path;
		material->GetTexture(type, i, &texture_path);

		textures.push_back(TexManager::getOrLoad(texture_path.C_Str(), tex_dir));	
	}
	return textures;
}
