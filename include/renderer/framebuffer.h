#pragma once

#include "common.h"
#include "asset_managers/texture_manager.h"

struct FrameBuffer {
    GLuint tex;

    int width, height;

    FrameBuffer(unsigned int width, unsigned int height) : width(width), height(height) {
        glGenFramebuffers(1, &FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);

        glGenRenderbuffers(1, &RBO);
        glBindRenderbuffer(GL_RENDERBUFFER, RBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

        tex = blankTexture(width, height);
	    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

        GLint last_FBO;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &last_FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, last_FBO);
        
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "FBO incomplete: " << status << '\n';
        }
    }
  	
    void bind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    }
    static void bindDefault() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

private:
    GLuint FBO;
    GLuint RBO;
};

struct ShadowFB {
    GLuint shadow_map;

    int width, height;

    ShadowFB(unsigned int width, unsigned int height) : width{width}, height{height} {
        glGenFramebuffers(1, &FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);

        shadow_map = blankTexture(width, height);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_map, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0); 
    }

private:
    GLuint FBO;
};

struct FBLevels {
    static inline const std::vector<std::pair<int, int>> level_to_res {
        { 1920, 1080 },
        { 1600, 900 } ,
        { 1280, 720 },
        { 960, 540 },
        { 640, 360 },
        { 320, 180 },
        { 160, 90 },
        { 80, 45 } 
    };

    std::vector<FrameBuffer> levels;

    FBLevels() {
        for (int i = 0; i < level_to_res.size(); ++i) {
            auto [width, height] = level_to_res.at(i);
            levels.emplace_back(FrameBuffer{ width, height });
        }
    }

    void bind(int level) {
        if (level < 0 || level >= levels.size()) {
            std::cout << "Invalid framebuffer level: " << level << '\n';
            return;
        }
        levels[level].bind();
    }

    FrameBuffer& getUsingWidth(int width) {
        auto it  = std::lower_bound(level_to_res.begin(), level_to_res.end(), width, 
        [](const auto& pair, int w) {
            return pair.first > w; 
        });

        if (it != level_to_res.end()) {
            int level = it - level_to_res.begin();
            bind(level);
            return levels[level];
        }
        return levels[0]; // Return the first framebuffer if no suitable level is found
    }
};
