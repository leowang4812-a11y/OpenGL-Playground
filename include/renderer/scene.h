#pragma once

#include "camera.h"
#include "uniform_buffer.h"
#include "model_manager.h"
#include "object.h"

struct Scene {
	Camera camera;
	std::vector<ObjectInstance> objects;
	std::vector<TerrainInstance> terrains;

	Transform transform;
	LightsUBO lUBO;

	Scene(const Camera& cam) : camera{cam}, objects{}, transform{}, lUBO{}, terrains{} {}

	static LightsUBO::Data testLights() {
		LightsUBO::Data data;
		data.num_dir_lights = 1;
		data.num_point_lights = 0;
		data.num_spotlights = 0;
		
		data.dir_lights[0].direction = glm::vec4{ -0.2f, -1.0f, -0.2f, 0.0f };
		data.dir_lights[0].ambient = glm::vec4{ 0.3f, 0.3f, 0.3f, 1.0f };
		data.dir_lights[0].diffuse = glm::vec4{ 0.4f, 0.4f, 0.4f, 1.0f };
		data.dir_lights[0].specular = glm::vec4{ 0.5f, 0.5f, 0.5f, 1.0f };

		return data;
	}
};


