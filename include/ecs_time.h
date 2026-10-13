#pragma once

#include "common.h"
#include "model.h"
#include "mesh.h"

class IComponent;
// (todo) eventually separate into a component based approach to allow for more flexible entities and to allow for more complex behaviour such as animation
// (TODO) eventually separate into a scene graph to allow for hierarchical transforms and to allow for more efficient rendering by allowing for frustum culling and occlusion culling
struct ModelInstance {
	std::string ID;

	Model* model;
	Transform transform;

	ModelInstance(std::string_view id, const AssetMap<Model>& models, Transform t)
		: ID{ id }
		, model{ models.at(ID).get() }
		, transform{ t } {}
};
// Engine
//  ├── Renderer
//  ├── TextureManager
//  ├── ShaderManager
//  ├── AssetManager
//  └── Scene

// Scene
//  └── entt::registry

// Entity
//  └── components

// Systems
//  ├── RenderSystem
//  ├── PhysicsSystem
//  ├── AnimationSystem
//  └── ScriptSystem
// struct DrawCommand
// {
//     Mesh* mesh;
//     Material* material;
//     glm::mat4 transform;
// };

// Then:

// std::vector<DrawCommand> commands;

// auto view = registry.view<
//     TransformComponent,
//     MeshRendererComponent
// >();

// for (auto e : view)
// {
//     auto& t = view.get<TransformComponent>(e);
//     auto& r = view.get<MeshRendererComponent>(e);

//     for (Mesh& mesh : r.model->meshes)
//     {
//         commands.push_back({
//             &mesh,
//             mesh.material,
//             ComputeMatrix(t)
//         });
//     }
// }