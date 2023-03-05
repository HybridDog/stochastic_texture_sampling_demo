#pragma once

#include <array>
#include <string>

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
//~ #include <GL/gl.h>


class ShaderProgram {
public:
	ShaderProgram(const std::string &path_vertex_shader, const std::string &path_fragment_shader);
	void use() const { glUseProgram(m_shader_program); }

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
	void setUniform(const std::string &name, std::array<float, 9> &m) const
		{ glUniformMatrix3fv(glGetUniformLocation(m_shader_program, name.c_str()),
			1, false, m.data()); };

private:
	GLuint m_shader_program{glCreateProgram()};
};
