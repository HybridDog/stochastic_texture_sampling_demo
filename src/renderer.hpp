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
	Renderer(SDL_Window *window);
	~Renderer() { glDeleteVertexArrays(1, &m_vao); }
	Renderer(const Renderer&) = delete;
	Renderer &operator=(const Renderer&) = delete;

	void render();
	void setImageLazy(const std::string &path);
	void setSize(int width, int height);
	Camera &getCamera() { return m_camera; }
	void setImageLazy(const std::string &path) const;
	void toggleInterpolation();
	void toggleColourTransformation() {
		m_colour_transformation_enabled = !m_colour_transformation_enabled; };

	// Specify that m_image_to_load is ready to be loaded
	void setImageLoaded() { m_image_ready = true; }
	std::string &getLoadingImage() { return m_image_to_load; }

private:
	// Set the portion of the frame which is rendered to.
	// pos1 and pos2 are in [-1, 1]^2
	void setRenderRect(const std::array<float, 2> &pos1,
		const std::array<float, 2> &pos2);

	int m_width{0};
	int m_height{0};
	bool m_interpolation_enabled{false};
	bool m_colour_transformation_enabled{true};

	// If true, the image file m_image_to_load is loaded and put into the
	// textures before the next frame is rendered
	bool m_image_ready{false};
	std::string m_image_to_load{""};

	Camera m_camera{Camera()};
	GLuint m_vao{0};
	ShaderProgram m_shader_program_default_sampling{
		DATA_PATH "/shaders/vertex.glsl",
		DATA_PATH "/shaders/fragment_default_sampling.glsl"};
	ShaderProgram m_shader_program_stochastic_sampling{
		DATA_PATH "/shaders/vertex.glsl",
		DATA_PATH "/shaders/fragment_stochastic_sampling.glsl"};
	Texture m_texture{false};
	TextureStochastic m_texture_stochastic{false};
};
