#include <iostream>
#include <array>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "common.hpp"
#include "renderer.hpp"


#ifdef __EMSCRIPTEN__
namespace {

Renderer *renderer_current;
void on_file_decoded(const char *)
{
	renderer_current->setImageLoaded();
}
void on_file_decoded_error(const char *)
{
	std::cerr << "Could not decode image " <<
		renderer_current->getLoadingImage() << "\n";
	// FIXME: delete the file here to free memory
}

}  // namespace
#endif


Renderer::Renderer(SDL_Window *window)
{
	int w, h;
	SDL_GetWindowSize(window, &w, &h);
	setSize(w, h);
	setImageLazy(DATA_PATH "/help_image.png");

	glGenVertexArrays(1, &m_vao);
}

void Renderer::setImageLazy(const std::string &path)
{
	m_image_to_load = path;
#ifdef __EMSCRIPTEN__
	m_image_ready = false;
	// With emscripten wait until the browser has decoded the image
	renderer_current = this;
	emscripten_run_preload_plugins(path.c_str(), on_file_decoded,
		on_file_decoded_error);
#else
	setImageLoaded();
#endif
}

void Renderer::setSize(int width, int height)
{
	m_width = width;
	m_height = height;
}

void Renderer::toggleInterpolation()
{
	m_interpolation_enabled = !m_interpolation_enabled;
	m_texture.setInterpolation(m_interpolation_enabled);
	m_texture_stochastic.setInterpolation(m_interpolation_enabled);
}

void Renderer::setRenderRect(const std::array<float, 2> &pos1,
	const std::array<float, 2> &pos2)
{
	glBindVertexArray(m_vao);
	std::array<f32, 20> vertices_triangle{
		pos1[0], pos1[1], 0.0f,
		pos2[0], pos1[1], 0.0f,
		pos1[0], pos2[1], 0.0f,
		pos2[0], pos2[1], 0.0f
	};
	// Vertex Buffer Object
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);  // bind stored in vao
	glBufferData(GL_ARRAY_BUFFER, vertices_triangle.size() * sizeof(f32),
		vertices_triangle.data(), GL_STATIC_DRAW);
	// Specify how the bound buffer (vbo) is interpreted: index, size, type,
	// …, stride, …
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void*)0);
	glEnableVertexAttribArray(0);  // attribarray stored in vao
	glBindVertexArray(0);
}

void Renderer::render()
{
	// Lazy texture loading
	if (m_image_ready) {
		m_image_ready = false;
		try {
			ImageFile img{m_image_to_load};
			m_texture.loadImage(img);
			m_texture_stochastic.loadImage(img);
		} catch (std::runtime_error e) {
			std::cerr << e.what() << std::endl;;
		}
		m_image_to_load = "";
		// FIXME: The file is never deleted
	}

	// Render the output to screen
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, m_width, m_height);
	//~ glClearColor(0.2f, 0.5f, 0.3f, 1.0f);
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	std::array<float, 2> texture_resolution{
		static_cast<float>(m_texture.getWidth()),
		static_cast<float>(m_texture.getHeight())
	};
	std::array<float, 2> cam_pos{m_camera.getPos()};

	// Draw with usual texture sampling
	m_shader_program_default_sampling.use();
	setRenderRect({-1.0f, -1.0f}, {0.0f, 1.0f});
	glBindVertexArray(m_vao);
	glActiveTexture(GL_TEXTURE0);
	m_texture.bind();
	m_shader_program_default_sampling.setUniform("myTexture", 0);
	m_shader_program_default_sampling.setUniform("textureResolution",
		texture_resolution);
	//~ m_shader_program_default_sampling.setUniform("pos0", m_camera.getPos());
	m_shader_program_default_sampling.setUniform("pos0", cam_pos);
	m_shader_program_default_sampling.setUniform("scale", m_camera.getZoom());
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// Draw with stochastic texture sampling
	m_shader_program_stochastic_sampling.use();
	setRenderRect({0.0f, -1.0f}, {1.0f, 1.0f});
	glBindVertexArray(m_vao);
	glActiveTexture(GL_TEXTURE0);
	if (m_colour_transformation_enabled) {
		m_texture_stochastic.bind();
	} else {
		m_texture.bind();
	}
	m_shader_program_stochastic_sampling.setUniform("myTexture", 0);
	glActiveTexture(GL_TEXTURE0 + 1);
	m_texture_stochastic.bind_lut();
	m_shader_program_stochastic_sampling.setUniform("colorLUT", 1);
	m_shader_program_stochastic_sampling.setUniform("textureResolution",
		texture_resolution);
	m_shader_program_stochastic_sampling.setUniform("pos0", cam_pos);
	m_shader_program_stochastic_sampling.setUniform("scale", m_camera.getZoom());
	m_shader_program_stochastic_sampling.setUniform("inverseDecorrelation",
		m_texture_stochastic.getInverseDecorrelation());
	m_shader_program_stochastic_sampling.setUniform("interpolationEnabled",
		m_interpolation_enabled);
	m_shader_program_stochastic_sampling.setUniform("gridScaling",
		m_grid_scaling);
	m_shader_program_stochastic_sampling.setUniform(
		"colourTransformationEnabled", m_colour_transformation_enabled);
	m_shader_program_stochastic_sampling.setUniform(
		"channelVisualisationEnabled",
		m_channel_visualisation_enabled);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
