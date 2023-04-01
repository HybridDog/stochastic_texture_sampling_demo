#pragma once

#include <array>
#include <string>

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>


class ShaderProgram {
public:
	// Load and compile a vertex and fragment shader from the file system
	ShaderProgram(const std::string &path_vertex_shader,
		const std::string &path_fragment_shader);
	~ShaderProgram() { glDeleteProgram(m_shader_program); };
	ShaderProgram(const ShaderProgram&) = delete;
	ShaderProgram &operator=(const ShaderProgram&) = delete;

	// Select the shader program for rendering
	void use() const { glUseProgram(m_shader_program); }

	// Overloaded function to set a uniform of the shader
	void setUniform(const std::string &name, float value) const
		{ glUniform1f(glGetUniformLocation(m_shader_program, name.c_str()),
			value); };
	void setUniform(const std::string &name, int value) const
		{ glUniform1i(glGetUniformLocation(m_shader_program, name.c_str()),
			value); };
	void setUniform(const std::string &name, bool value) const
		{ glUniform1i(glGetUniformLocation(m_shader_program, name.c_str()),
			value); };
	void setUniform(const std::string &name, std::array<float, 2> &vec) const
		{ glUniform2f(glGetUniformLocation(m_shader_program, name.c_str()),
			vec[0], vec[1]); };
	void setUniform(const std::string &name, std::array<float, 3> &vec) const
		{ glUniform3f(glGetUniformLocation(m_shader_program, name.c_str()),
			vec[0], vec[1], vec[2]); };
	void setUniform(const std::string &name, std::array<float, 9> &m) const
		{ glUniformMatrix3fv(glGetUniformLocation(m_shader_program, name.c_str()),
			1, false, m.data()); };

private:
	GLuint m_shader_program{glCreateProgram()};
};
