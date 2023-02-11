#include <iostream>
#include <array>

#include "common.hpp"
#include "renderer.hpp"


void Renderer::setSize(int width, int height)
{
	m_width = width;
	m_height = height;
}

Renderer::Renderer(int width, int height)
{
	setSize(width, height);

	// Vertex Array Object; associates attribointers and VBOs
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	std::array<f32, 20> vertices_triangle{
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		1.0f,  1.0f, 0.0f
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
	//~ std::array<float, 2> invViewPortSize{1.0f / m_width, 1.0f / m_height};

	// Render the output to screen
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, m_width, m_height);
	glClearColor(0.2f, 0.5f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	m_shader_program_default_sampling.use();
	glBindVertexArray(m_vao);
	glActiveTexture(GL_TEXTURE0);
	m_texture.bind();
	std::array<float, 2> texture_resolution{
		static_cast<float>(m_texture.getWidth()),
		static_cast<float>(m_texture.getHeight())
	};
	m_shader_program_default_sampling.setUniform("myTexture", 0);
	m_shader_program_default_sampling.setUniform("textureResolution",
		texture_resolution);
	std::array<float, 2> cam_pos{m_camera.getPos()};
	//~ m_shader_program_default_sampling.setUniform("pos0", m_camera.getPos());
	m_shader_program_default_sampling.setUniform("pos0", cam_pos);
	m_shader_program_default_sampling.setUniform("scale", m_camera.getZoom());
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
