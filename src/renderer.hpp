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

	// Render a frame
	void render();
	// Pass the size of the to-be-rendered frame
	void setSize(int width, int height);

	Camera &getCamera() { return m_camera; }
	// Set an image which is to be loaded and decoded once it is ready
	void setImageLazy(const std::string &path);

	// Methods to configure stochastic texture sampling parameters
	void toggleInterpolation();
	void toggleColourTransformation() {
		m_colour_transformation_enabled = !m_colour_transformation_enabled; }
	void toggleChannelVisualisation() {
		m_channel_visualisation_enabled =
		!m_channel_visualisation_enabled; }
	float getGridScaling() const { return m_grid_scaling; }
	void setGridScaling(float s) { m_grid_scaling = s; }

	// Callback helper functions to specify that m_image_to_load is ready to be
	// loaded and to get path of the current to-be-loaded image
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
	bool m_channel_visualisation_enabled{false};
	float m_grid_scaling{3.464f};  // 2 * sqrt(3)

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
