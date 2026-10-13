#pragma once
#include "shader.h"

class ShaderManager {
public:
	static Shader getOrLoad(const std::string& vert_shader, const std::string& frag_shader, const std::string& geom_shader="") {
		std::string key = vert_shader + frag_shader + geom_shader;
		auto it = m_cache.find(key);

		if (it != m_cache.end()) return it->second;
			
		Shader shader = Shader{vert_shader, frag_shader, geom_shader};

		m_cache[key] = shader;

		return m_cache[key];
	}
private:
	static inline std::unordered_map<std::string, Shader> m_cache{};
};
