// Engine
//  ├── Scene (entt::registry)
//  ├── Systems
//  │     ├── InputSystem
//  │     ├── CameraSystem
//  │     ├── PhysicsSystem
//  │     ├── AnimationSystem
//  │     ├── RenderSystem
//  └── Renderer (GPU abstraction)

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