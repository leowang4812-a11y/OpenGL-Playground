#pragma once

#include "common.h"
#include "uniform_buffer.h"
#include "model_manager.h"
#include "object.h"

namespace {
	static constexpr glm::vec3 START_POS{ 0.1f, 0.1f, 0.1f };
	static constexpr glm::vec3 DEFAULT_WORLD_UP{ 0.0f, 1.0f, 0.0f };

	static constexpr float START_YAW = -90.0f;
	static constexpr float START_PITCH = 0.0f;
	static constexpr float START_ROLL = 0.0f;
	static constexpr float MOUSE_SENSITIVITY = 0.1f;
	static constexpr float ROLL_SPEED = 30.0f;
	static constexpr float FOV_MAX = 90.0f;
	static constexpr float MOVESPEED = 10.0f;
	static constexpr float NEAR_PLANE = 0.1f;
	static constexpr float FAR_PLANE = 1000.0f;
}

#define debug(x) std::cerr << #x << " = " << (x) << std::endl

struct ViewBasis {
	glm::vec3 pos;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;

	glm::vec3 scale={1.0f, 1.0f, 1.0f};

	float near = NEAR_PLANE;
	float far = FAR_PLANE;
	float FOV = FOV_MAX;

	glm::mat4 getModelMatrix() const { 
		glm::mat4 trans = glm::translate(glm::mat4(1.0f), pos);
		glm::mat4 rot(1.0f);
		rot[0] = glm::vec4(right, 0.0f);
		rot[1] = glm::vec4(up, 0.0f); 
		rot[2] = glm::vec4(-front, 0.0f);

		glm::mat4 s = glm::scale(glm::mat4(1.0f), scale);

		return trans * rot * s;
	}

	glm::mat4 getViewMatrix() { 
		return glm::lookAt(pos, pos + front, up); 
	}

	glm::mat4 getProjMatrix(int width, int height) const {
		return glm::perspective(glm::radians(FOV), (float)width / (float)height, near, far);
	}
	void setUBO(const CameraUBO& UBO, int width, int height) {
		UBO.setData(CameraUBO::Data{ getViewMatrix(), getProjMatrix(width, height) });
	}

};

struct Camera {
	enum Movement {
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT,
		ROLL_LEFT,
		ROLL_RIGHT,
		SPEED_UP,
	};

	glm::quat q;
	ViewBasis basis;
	ObjectInstance object;

	uint32_t width = Settings::DEFAULT_VIEW_WIDTH, height = Settings::DEFAULT_VIEW_HEIGHT;

	float yaw = START_YAW;
	float pitch = START_PITCH;
	float roll = START_ROLL;
	float mouse_sens = MOUSE_SENSITIVITY;
	float roll_speed = ROLL_SPEED;
	float movespeed = MOVESPEED;

	// Camera(uint32_t view_width, uint32_t view_height, glm::vec3 start_pos, glm::vec3 up,
	// 	float yaw, float pitch, float sensitivity, float FOV, float movespeed, float near, float far) 
    //     : m_pos{ start_pos }, m_width{ view_width }, m_height{ view_height }, m_yaw{ yaw }, m_pitch{ pitch },
    //       m_mouse_sensitivity{ sensitivity }, m_FOV{ FOV }, m_movespeed{ movespeed }, m_near{ near }, m_far{ far } {
    //     updateCameraVectors();
    // }
	Camera(uint32_t view_width, uint32_t view_height, const ObjectInstance& object_instance) 
		: width{ view_width }
		, height{ view_height } 
		, q{1.0f, 0.0f, 0.0f, 0.0f} 
		, object{object_instance} {
		
		basis.pos = START_POS;
        updateCameraVectors();
    }
	void processKeyboard(Movement direction, float dt) {
		float vel_mag = movespeed * dt;

		switch (direction) {
		case Movement::FORWARD:
			basis.pos += basis.front  * vel_mag;
			break;
		case Movement::BACKWARD:
			basis.pos -= basis.front  * vel_mag;
			break;
		case Movement::LEFT:
			basis.pos -= basis.right  * vel_mag;
			break;
		case Movement::RIGHT:
			basis.pos += basis.right  * vel_mag;
			break;
		case Movement::ROLL_LEFT:
			roll -= roll_speed * dt;
			break;
		case Movement::ROLL_RIGHT:
			roll += roll_speed * dt;
			break;
		case Movement::SPEED_UP: 
			movespeed = MOVESPEED * 10.0f;
			break;
		}

		if (direction != Movement::SPEED_UP) movespeed = MOVESPEED;

		updateCameraVectors();
	}

	// idx of mesh coord x = width/2.0f is at 0.0f
	// idx of mesh coord y = height/2.0f is at 0.0f, going downwards
	// map camera coordinate between two mesh coords in the x and y directions. Span a plane with these two vectors and sample the x and y positions of the camera onto the plane
	// then get the coressponding z coordinate.
	// assume terrain not rotated
	void terrainInteraction(std::vector<TerrainInstance>& instances) {
		glm::vec3 cam_pos = basis.pos;


		for (auto& terrain : instances) {
			glm::vec3 model_cam_pos = cam_pos;
			Transform transform = terrain.transform;
			glm::vec3 scale = transform.scale;
			glm::mat3 model_m = glm::mat3(transform.computeModelMatrix());

			model_cam_pos.x /= scale.x;
			model_cam_pos.z /= scale.z;

			int map_height = terrain.terrain_mesh->height;
			int map_width  = terrain.terrain_mesh->width;

			int X1 = static_cast<int>(std::floor(model_cam_pos.x + map_width / 2.0f));
			int X2 = static_cast<int>(std::ceil(model_cam_pos.x + map_width / 2.0f));
			
			int Y1 = static_cast<int>(std::floor(model_cam_pos.z + map_height / 2.0f));
			int Y2 = static_cast<int>(std::ceil(model_cam_pos.z + map_height / 2.0f));		

			// get 3 points:


			auto& vertices = terrain.terrain_mesh->mesh.m_vertices;

			int idx1 = Y1 * map_width + X1;
			int idx2 = Y2 * map_width + X1;
			int idx3 = Y2 * map_width + X2;

			if (idx1 == idx2) idx1 = (Y1 - 1) * map_width + X1;
			if (idx2 == idx3) ++idx3;

			glm::vec3 vert1 = model_m * vertices[idx1].pos;
			glm::vec3 vert2 = model_m * vertices[idx2].pos;
			glm::vec3 vert3 = model_m * vertices[idx3].pos;

			glm::vec3 norm = glm::normalize(glm::cross(vert1 - vert2, vert2 - vert3));

			float D = norm.x * vert1.x + norm.y * vert1.y + norm.z * vert1.z;

			// now we have a full plane in world space. sample 
			float y = (D - norm.x * cam_pos.x - norm.z * cam_pos.z) / norm.y;

			if (basis.pos.y < y + 0.5f) basis.pos.y = y + 0.5f;
		}
	}

	void applyPhysicsEffects() {

	}

	void processMouseMovement(float xoffset, float yoffset) {
		yaw   += xoffset * mouse_sens;
		pitch += yoffset * mouse_sens ;

		if      (pitch > 89.0f)  pitch = 89.0f;
		else if (pitch < -89.0f) pitch = -89.0f;

		updateCameraVectors();
	}
	void processMouseScroll(float yoffset) {
		basis.FOV -= yoffset;
		if (basis.FOV < 1.0f) basis.FOV = 1.0f;
		if (basis.FOV > FOV_MAX) basis.FOV = FOV_MAX;
	}

	void updateCameraVectors() {
		q = glm::quat(glm::vec3(glm::radians(pitch), glm::radians(-yaw), glm::radians(-roll)));
		basis.front = glm::rotate(q, { 0.0, 0.0, -1.0 });
		basis.up    = glm::rotate(q, { 0.0, 1.0,  0.0 });
		basis.right = glm::rotate(q, { 1.0, 0.0,  0.0 });
	}

	void setViewportSize(uint32_t new_width, uint32_t new_height) {
		width  = new_width;
		height = new_height;
	}

	void setBasisVectors(glm::vec3 front, glm::vec3 up, glm::vec3 right) {
		basis.right = right;
		basis.up = up;
		basis.front = front;

		glm::mat3 m(right, up, front);

		q = glm::quat_cast(m);
		glm::vec3 euler = glm::eulerAngles(q);
		pitch = glm::degrees(euler.x);
		yaw = glm::degrees(euler.y);
		roll = glm::degrees(euler.z);
	}

	// this is for when the camera may be drawn externally.
	glm::mat4 getModelMatrix() const { return basis.getModelMatrix(); }
	glm::mat4 getViewMatrix() { return basis.getViewMatrix(); }
	glm::mat4 getProjMatrix() const { return basis.getProjMatrix(width, height); }

	void setUBOData(const CameraUBO& UBO) {
		basis.setUBO(UBO, width, height);
	}	
};

