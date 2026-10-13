#define GL_SILENCE_DEPRECATION
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <random>

#include "common.h"
#include "camera.h"
#include "renderer/renderer.h"
#include "renderer/mesh.h"
#include "renderer/material.h"
#include "renderer/shader.h"
#include "renderer/scene.h"
#include "renderer/framebuffer.h"
#include "renderer/mirror.h"
#include "renderer/object.h"

#include "asset_managers/model_manager.h"
#include "asset_managers/shader_manager.h"

 
#define debug(x) std::cerr << #x << " = " << (x) << std::endl

void frameBufferSizeCallback(GLFWwindow* window, int width, int height) {

	if (width == 0 || height == 0)
		return;

	Camera* active_camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
	active_camera->width = width;
	active_camera->height = height;

	if (active_camera) {
		active_camera->setViewportSize(width, height);
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	static float mouse_xlast = Settings::DEFAULT_VIEW_WIDTH / 2.0f;
	static float mouse_ylast = Settings::DEFAULT_VIEW_HEIGHT / 2.0f;
	static bool first_opened_mouse = false;

	if (first_opened_mouse) {
		mouse_xlast = xpos;
		mouse_ylast = ypos;
		first_opened_mouse = false;
	}
	float xoffset = xpos - mouse_xlast;
	float yoffset = mouse_ylast - ypos;

	mouse_xlast = xpos;
	mouse_ylast = ypos;

	Camera* active_camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
	active_camera->processMouseMovement(xoffset, yoffset);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	Camera* active_camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
	active_camera->processMouseScroll(yoffset);
}

class Platform {
public:
    Platform() {
       	glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		window = glfwCreateWindow(Settings::DEFAULT_VIEW_WIDTH, Settings::DEFAULT_VIEW_HEIGHT, "opengl", NULL, NULL);

		if (window == NULL) {
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
		}

		glfwMakeContextCurrent(window);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // hides the cursor and keeps it in the centre for mouse active_camera rotation

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			std::cout << "Failed to initialize GLAD" << std::endl;
		} 
    }
    ~Platform() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
    GLFWwindow* getWindow() const { return window; }

private:
    GLFWwindow* window;
};

struct Knob {
	float val=0.0f;
	float speed=0.0f;
};

void processInput(Camera& active_camera, GLFWwindow* window, float dt, std::array<Knob, 10>& knobs) {
	if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		active_camera.processKeyboard(Camera::Movement::FORWARD, dt);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		active_camera.processKeyboard(Camera::Movement::BACKWARD, dt);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		active_camera.processKeyboard(Camera::Movement::LEFT, dt);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		active_camera.processKeyboard(Camera::Movement::RIGHT, dt);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		active_camera.processKeyboard(Camera::Movement::ROLL_LEFT, dt);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		active_camera.processKeyboard(Camera::Movement::ROLL_RIGHT, dt);
	
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		active_camera.processKeyboard(Camera::Movement::SPEED_UP, dt);
	}

	int sign = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ? -1 : 1;
	for (int i = 0; i < knobs.size(); ++i) {
		if (glfwGetKey(window, GLFW_KEY_0 + i) == GLFW_PRESS) {
			knobs[i].val += knobs[i].speed * dt * sign;
		}
	}
}


int main() {
    Platform platform{};
	
    GLFWwindow* window = platform.getWindow();
	
	std::vector<ModelInstance> cube = ModelManager::loadInstances("assets/objects/cube/cube.obj");

	TerrainMesh terrain_mesh = std::move(terrainGen());
	Transform terrain_transform{};
	terrain_transform.pos = glm::vec3{ 0.0f, 0.0f, 0.0f };

    Camera camera(100, 100, ObjectInstance{ cube, Transform{} });
	Scene scene{ camera };
	scene.terrains.emplace_back(TerrainInstance{ &terrain_mesh, terrain_transform });

	for (auto vertex : scene.terrains[0].terrain_mesh->mesh.m_vertices) {
		//std::cout << vertex.pos.x << ", " << vertex.pos.y << ", " << vertex.pos.z << '\n';
	}

	Camera& active_camera = scene.camera;

	// std::vector<ModelInstance> main_sponza = ModelManager::loadInstances("assets/objects/main_sponza/NewSponza_Main_glTF_003.gltf");
	// std::vector<ModelInstance> curtain_sponza = ModelManager::loadInstances("assets/objects/pkg_a_curtains/NewSponza_Curtains_glTF.gltf");
	// scene.m_models.insert(scene.m_models.end(), main_sponza.begin(), main_sponza.end());
	// scene.m_models.insert(scene.m_models.end(), curtain_sponza.begin(), curtain_sponza.end());
	std::vector<ModelInstance> plane = ModelManager::loadInstances("assets/objects/plane/plane.obj");
	scene.objects.push_back(ObjectInstance{ plane, Transform{} });

	std::vector<ModelInstance> donut = ModelManager::loadInstances("assets/objects/donut/donut.obj");
	scene.objects.push_back(ObjectInstance{ donut, Transform{} });
	
	scene.objects.push_back(ObjectInstance{ cube, Transform{ .pos = glm::vec3{10.0f, 0.0f, -10.0f} } });
	

	std::vector<ModelInstance> house = ModelManager::loadInstances("assets/objects/test/test.obj");

	scene.objects.push_back(ObjectInstance{ house, Transform{ .pos = glm::vec3{-15.0f, 0.0f, -200.0f} } });

	std::vector<ModelInstance> mug = ModelManager::loadInstances("assets/objects/mug/mug.obj");

	scene.objects.push_back(ObjectInstance{ mug, Transform{ .pos = glm::vec3{-15.0f, 0.0f, -20.0f}, .scale = {10.f, 10.0f, 10.0f } } });

    Renderer renderer{};
	CameraUBO UBO;

	std::array<Knob, 10> knobs;

    glfwSetWindowUserPointer(window, &active_camera);
    glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	auto primary_monitor_video_mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	active_camera.width = primary_monitor_video_mode->width;
	active_camera.height = primary_monitor_video_mode->height;

	glViewport(0, 0, active_camera.width, active_camera.height);

	glfwSetWindowMonitor(
		window,
		glfwGetPrimaryMonitor(),
		0, 0,
		primary_monitor_video_mode->width,
		primary_monitor_video_mode->height,
		primary_monitor_video_mode->refreshRate
	);

	FrameBuffer screen_FBO{ active_camera.width, active_camera.height };

	Mesh quad_mesh = screenQuad();
	Mirror mirror{ active_camera };

	Shader post = ShaderManager::getOrLoad("assets/shaders/post.vert", "assets/shaders/post.frag");


    Shader basic_shader = ShaderManager::getOrLoad("assets/shaders/basic.vert", "assets/shaders/basic.frag", "assets/shaders/basic.geom");

	knobs[1].speed = 10.0f;
	knobs[1].val = 0.0f;

	knobs[2].speed = 10.0f;
	knobs[2].val = 0.0f;

	knobs[3].speed = 10.0f;
	knobs[3].val = 0.0f;

	std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 100);

	glm::vec4 clear_colour = glm::vec4(0.3f, 0.5f, 0.7f, 1.0f);

	float dt;
	float last_frame_time = 0.0f;
	int frame_index = 0;
    while (!glfwWindowShouldClose(window)) {
		float current_frame_time = glfwGetTime();
		dt = current_frame_time - last_frame_time;
		last_frame_time = current_frame_time;

		post.setFloat("time_elapsed", current_frame_time);

		int next_frame_index = (int)current_frame_time;
		if (next_frame_index != frame_index) {
			frame_index = next_frame_index;
			std::cout << "Camera position: " << active_camera.basis.pos.x << ", " << active_camera.basis.pos.y << ", " << active_camera.basis.pos.z << '\n';
			std::cout << "Camera yaw: " << active_camera.yaw << ", pitch: " << active_camera.pitch << '\n';
			std::cout << "FPS: " << 1.0 / dt<< '\n';
			std::cout << "Camera width and height: " << active_camera.width << ' ' << active_camera.height << '\n';	

			// per second update.
		}


		// auto collides = [](const Camera& active_camera, const Mirror& mirror) -> std::pair<bool, bool> {
		// 	auto& verts = mirror.mesh.m_vertices;
		// 	auto mat = glm::mat3(mirror.basis.getModelMatrix());

		// 	glm::vec3 u = mat * glm::normalize(verts[0].pos - verts[1].pos); // vert
		// 	glm::vec3 v = mat * glm::normalize(verts[0].pos - verts[5].pos); // hori

		// 	glm::vec3 cam_to_mir = mirror.basis.pos - active_camera.basis.pos;

		// 	float dot1 = glm::abs(glm::dot(u, cam_to_mir));
		// 	float dot2 = glm::abs(glm::dot(v, cam_to_mir));
		// 	float norm_dist = glm::dot(mirror.basis.front, cam_to_mir);

		// 	return { (glm::abs(norm_dist) < 0.1f && dot1 < mirror.basis.scale.y && dot2 < mirror.basis.scale.x), (norm_dist > 0) };
		// };
		// static bool intersects = false;
		// static bool is_pos = true;
		// auto [collision, is_positive] = collides(active_camera, mirror);


		// if (collision) {
		// 	if (!intersects) {
		// 		is_pos = is_positive;
		// 	}
		// 	intersects = true;
		// }
		// else {
		// 	if (intersects && is_pos != is_positive) {
		// 		is_pos = is_positive;
		// 		std::swap(mirror.clear_colour, clear_colour);
		// 		ViewBasis reflec_basis = mirror.getReflectedViewBasis(active_camera);
		// 		active_camera.setBasisVectors(reflec_basis.front, reflec_basis.up, reflec_basis.right);
		// 	}
		// 	intersects = false;
		// }


		processInput(active_camera, window, dt, knobs);
		active_camera.terrainInteraction(scene.terrains); 

//////////////////
		ViewBasis reflec_basis = mirror.getReflectedViewBasis(active_camera);

		mirror.prepareRenderCapture(UBO, active_camera, reflec_basis);
        renderer.render(scene, reflec_basis, current_frame_time);
		renderer.renderCamera(scene.camera, reflec_basis);

/////////////////////////////
		glViewport(0, 0, active_camera.width, active_camera.height);
		screen_FBO.bind();
		active_camera.setUBOData(UBO);

        glClearColor(clear_colour.x, clear_colour.y, clear_colour.z, clear_colour.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		mirror.draw(UBO, active_camera, reflec_basis);

		basic_shader.use();
        renderer.render(scene, active_camera.basis, current_frame_time);
		renderer.testRender(active_camera.basis);

		
////////////////////////////////////////////
		FrameBuffer::bindDefault();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		post.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, screen_FBO.tex);
		post.setInt("screen_texture", 0);
		post.setFloat("time_elapsed", current_frame_time);
		post.setVec2("texel_size", glm::vec2{ 1.0 / (float)screen_FBO.width, 1.0 / (float)screen_FBO.height });

		quad_mesh.draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    return 0;
}

