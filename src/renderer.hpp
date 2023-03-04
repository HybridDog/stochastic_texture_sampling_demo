#pragma once

#include <GL/glew.h>

#include "shader_program.hpp"
#include "texture.hpp"
#include "texture_stochastic.hpp"
#include "camera.hpp"
#include "image_file.hpp"

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
	void setImage(const ImageFile &img) { m_texture.loadImage(img);
		m_texture_stochastic.loadImage(img); }

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
	Texture m_texture{Texture{ImageFile{DATA_PATH "/sumpf_cobble.png"}, false}};
	TextureStochastic m_texture_stochastic{
		TextureStochastic{ImageFile{DATA_PATH "/sumpf_cobble.png"}, false}};
};
