#pragma once

#include "mesh.h"
#include "camera.h"
#include "framebuffer.h"
#include "shader_manager.h"
#define debug(x) std::cerr << #x << " = " << (x) << std::endl

namespace {
    glm::vec3 reflectPosOverPlane(glm::vec3 mirror_pos, glm::vec3 camera_pos, glm::vec3 mirror_normal) {
        mirror_normal = glm::normalize(mirror_normal);
        glm::vec3 to_mirror = mirror_pos - camera_pos;
        glm::vec3 n_comp = mirror_normal * glm::dot(to_mirror, mirror_normal);

        return 2.0f * n_comp + camera_pos;
    }

}

struct Mirror {
    Mesh mesh;
    ViewBasis basis;
    FBLevels FB_level;

    FrameBuffer* cur_FBO = nullptr;

    Shader normal_shader;
    Shader mirror_render_shader;

    glm::vec4 clear_colour={0.8f, 0.7f, 0.2f, 1.0f};

    Mirror(const Camera& camera) 
        : mesh{ screenQuad() } 
        , FB_level{} {

        normal_shader = ShaderManager::getOrLoad("assets/shaders/basic.vert", "assets/shaders/basic.frag", "assets/shaders/basic.geom");
        mirror_render_shader = ShaderManager::getOrLoad("assets/shaders/mirror.vert", "assets/shaders/mirror.frag");

        glm::quat rot = glm::angleAxis(glm::radians(135.0f), glm::vec3(0, 1, 0));
        glm::mat3 rot_m = glm::mat3_cast(rot);
        
        basis.right = rot_m * glm::vec3(-1.0f, 0.0f, 0.0f);
        basis.up = rot_m * glm::vec3(0.0f, 1.0f, 0.0f);
        basis.front = rot_m * glm::vec3(0.0f, 0.0f, 1.0f);

        debug(rot.x);
        debug(rot.y);
        debug(rot.z);
        debug(rot.w);  

        basis.pos = glm::vec3(15.0f, 0.0f, 0.0f);

        basis.scale = glm::vec3(5.0f, 5.0f, 5.0f);
    }
    
    ViewBasis getReflectedViewBasis(const Camera& camera) {
        ViewBasis reflec_view = camera.basis;
		reflec_view.pos = reflectPosOverPlane(basis.pos, camera.basis.pos, basis.front);
		reflec_view.front = glm::reflect(camera.basis.front, basis.front);
		reflec_view.up =    glm::reflect(camera.basis.up, basis.front);
		reflec_view.right = glm::reflect(camera.basis.right, basis.front);

        return reflec_view;
    }

    void prepareRenderCapture(const CameraUBO& UBO, const Camera& camera, ViewBasis& reflec_basis) {
        float full_width = glm::length(camera.basis.pos - basis.pos) * glm::tan(glm::radians(camera.basis.FOV/2.0f)) * 2.0f;
        float mirror_width = basis.scale.x * 2.0f;
        float width_ratio = mirror_width / full_width;
        int width_coverage = static_cast<int>(width_ratio * camera.width) * 2;
        
        cur_FBO = &FB_level.getUsingWidth(width_coverage); // Bind the appropriate framebuffer level
        cur_FBO->bind();
        glClearColor(clear_colour.x, clear_colour.y, clear_colour.z, clear_colour.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		reflec_basis.setUBO(UBO, cur_FBO->width, cur_FBO->height);
        glViewport(0, 0, cur_FBO->width, cur_FBO->height);
    }

    void draw(const CameraUBO& UBO, Camera& camera, ViewBasis& reflec_basis) {
        mirror_render_shader.use();

		mirror_render_shader.setInt("mirror.tex", 0);
		mirror_render_shader.setFloat("mirror.min_reflectivity", 0.5f);
		mirror_render_shader.setFloat("mirror.exponent", 4.0f);
		mirror_render_shader.setVec3("view_pos", camera.basis.pos);

		mirror_render_shader.setMat4("mirror_view_proj", reflec_basis.getProjMatrix(camera.width, camera.height) * reflec_basis.getViewMatrix());
		mirror_render_shader.setMat4("model", basis.getModelMatrix());
        
		camera.setUBOData(UBO);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cur_FBO->tex);

        mesh.draw();
    }
};