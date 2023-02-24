#pragma once

#include <GL/glew.h>

#include "shader_program.hpp"
#include "texture.hpp"
#include "camera.hpp"

#ifdef __EMSCRIPTEN__
#define DATA_PATH "/data"
#else
#define DATA_PATH "./data"
#endif


class Renderer {
public:
	Renderer(int width, int height);
	void render();
	void setSize(int width, int height);
	Camera &getCamera() { return m_camera; }

private:
	// Set the portion of the frame which is rendered to.
	// pos1 and pos2 are in [-1, 1]^2
	void setRenderRect(const std::array<float, 2> &pos1,
		const std::array<float, 2> &pos2);

	int m_width{0};
	int m_height{0};
	Camera m_camera{Camera()};
	GLuint m_vao{0};
	ShaderProgram m_shader_program_default_sampling{
		DATA_PATH "/shaders/vertex.glsl",
		DATA_PATH "/shaders/fragment_default_sampling.glsl"};
	ShaderProgram m_shader_program_stochastic_sampling{
		DATA_PATH "/shaders/vertex.glsl",
		DATA_PATH "/shaders/fragment_stochastic_sampling.glsl"};
	Texture m_texture{Texture(DATA_PATH "/moontest_stone.png")};
};
