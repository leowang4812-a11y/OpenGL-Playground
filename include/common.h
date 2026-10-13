#pragma once

namespace Settings {
	constexpr int DEFAULT_VIEW_WIDTH  = 800;
	constexpr int DEFAULT_VIEW_HEIGHT = 600;
}

template <typename T>
using AssetMap = std::unordered_map<std::string, std::shared_ptr<T>>;

struct Transform {
	glm::mat4 computeModelMatrix() const {
		glm::mat4 T = glm::translate(glm::mat4(1.0f), pos);
		glm::mat4 R = glm::toMat4(quat);
		glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
		return T * R * S;
	}

	static Transform combine(const Transform& a, const Transform& b) {
		Transform result;
		result.scale = a.scale * b.scale;
		result.quat = a.quat * b.quat; // (TODO) normalise occasionally in case of floating point errors.
		result.pos = a.pos + a.quat * (a.scale * b.pos);

		return result;
	}

	glm::vec3 pos  { 0.0f, 0.0f, 0.0f };
	glm::quat quat { 1.0f, 0.0f, 0.0f, 0.0f };
	glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
};


struct Vertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 tex_coords;
};

