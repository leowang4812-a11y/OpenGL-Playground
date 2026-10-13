#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static std::tuple<unsigned char*, int, int, int> getImgData(const std::string& path) {
	std::cout << "HERE\n";
	int width, height, nr_channels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nr_channels, 0);
	if (!data) {
		std::cout << "Failed to load texture" << '\n';
		std::cout << "Failed path: " << path << '\n';
	}

	return std::make_tuple(data, width, height, nr_channels);
}

static void freeImgData(unsigned char* data) {
	stbi_image_free(data);
}

static GLuint loadTexture(const std::string& path, const std::string& directory) {
	std::string filename = directory + '/' + path;

	GLuint texture_ID;
	glGenTextures(1, &texture_ID);

	int width, height, nr_channels;

	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nr_channels, 0);
	if (data) {
		GLenum channels_format;
		switch (nr_channels) {
		case 1:
			channels_format = GL_RED;
			break;
		case 3:
			channels_format = GL_RGB;
			break;
		case 4:
			channels_format = GL_RGBA;
			break;
		default:
			std::cout << "Load Texture Fail: Invalid number of channels\n";
			return -1;
		}
		glBindTexture(GL_TEXTURE_2D, texture_ID);
		glTexImage2D(GL_TEXTURE_2D, 0, channels_format, width, height, 0, channels_format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else {
		std::cout << "Failed to load texture" << '\n';
		std::cout << "Failed path: " << path << '\n';
		std::cout << "Directory: " << directory << '\n';
	}

	return texture_ID;
}

static GLuint load1x1TextureRGB(glm::vec3 c) {
	unsigned char pixel[3] = {
		static_cast<unsigned char>(glm::clamp(c.r, 0.0f, 1.0f) * 255.0f),
		static_cast<unsigned char>(glm::clamp(c.g, 0.0f, 1.0f) * 255.0f),
		static_cast<unsigned char>(glm::clamp(c.b, 0.0f, 1.0f) * 255.0f)
	};

	GLuint texture_ID;
	glGenTextures(1, &texture_ID);
	glBindTexture(GL_TEXTURE_2D, texture_ID);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGB8,
		1, 1,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		pixel
	);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return texture_ID;
}


static GLuint blankTexture(unsigned int width, unsigned int height) {
	GLuint ID;	
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_2D, ID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
	
	return ID;
}
struct Texture {
	unsigned int ID;
	std::string path;
};

class TexManager {
public:
	static Texture getOrLoad(const std::string& tex_path, const std::string& tex_dir) {
		auto it = m_cache.find(tex_path);

		if (it != m_cache.end()) return it->second;

		Texture texture{
			loadTexture(tex_path, tex_dir),
			tex_path
		};

		m_cache.emplace(tex_path, texture);
		return texture;
	}
	static Texture getOrCreateSolidColour(glm::vec4 colour) {
		std::string key{ std::to_string(colour.x) + std::to_string(colour.y) + std::to_string(colour.z) + ".none"};

		auto it = m_cache.find(key);
		if (it != m_cache.end()) return it->second;
		
		Texture texture{
			load1x1TextureRGB(colour),
			key
		};

		m_cache.emplace(key, texture);
		return texture;
	}

	static Texture getDefaultWhite() { return getOrCreateSolidColour( { 1.0f, 1.0f, 1.0f, 1.0f } ); }


	static bool isOpaque(const Texture& tex) {
		std::size_t dot = tex.path.find_last_of('.');

		if (dot == std::string_view::npos) return true;

		std::string_view extension = std::string_view(tex.path).substr(dot + 1);

		if (extension == "none") return true;

		if (extension == "jpg" || extension == "JPG" || 
			extension == "jpeg" || extension == "JPEG") return true;
		
		return false; // assume always transparent if png
	}

private:
	static inline std::unordered_map<std::string, Texture> m_cache{};
};

