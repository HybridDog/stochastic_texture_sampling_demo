#pragma once

#include <GL/glew.h>

#include "shader_program.hpp"
#include "texture.hpp"
#include "camera.hpp"


class Renderer {
public:
	Renderer(int width, int height);
	void render();
	void setSize(int width, int height);
	Camera &getCamera() { return m_camera; }

private:
	int m_width{0};
	int m_height{0};
	Camera m_camera{Camera()};
	GLuint m_vao{0};
	//~ ShaderProgram m_shader_program_default_sampling{PROJECT_DIRECTORY "/shaders/vertex.glsl", PROJECT_DIRECTORY "/shaders/fragment_init.glsl"};
#ifdef __EMSCRIPTEN__
	ShaderProgram m_shader_program_default_sampling{"/data/shaders/vertex.glsl", "/data/shaders/fragment_default_sampling.glsl"};
	Texture m_texture{Texture("/data/moontest_stone.png")};
#else
	ShaderProgram m_shader_program_default_sampling{"data/shaders/vertex.glsl", "data/shaders/fragment_default_sampling.glsl"};
	Texture m_texture{Texture("data/moontest_stone.png")};
#endif
};
