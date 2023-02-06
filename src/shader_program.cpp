#include <iostream>
#include <fstream>

#include "shader_program.hpp"


namespace {

// Read the whole content of a file into a string or throw an exception on
// failure
std::string read_file(const std::string &path)
{
	if (std::ifstream infile{path, std::ios::binary | std::ifstream::ate}) {
		auto size{infile.tellg()};
		std::string str(size, '\0');
		infile.seekg(0);
		if (infile.read(&str[0], size))
			return str;
	};
	throw std::runtime_error("Could not load file: " + path);
}

// Check if the shader has compiled successfully
bool check_shader_compile_status(const GLuint &shader,
	const std::string &name)
{
	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLint info_log_len;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_len);
		char info_log[info_log_len];
		glGetShaderInfoLog(shader, info_log_len, nullptr, info_log);
		std::cerr << "Shader compilation failed (" << name << ")\n" <<
			info_log << std::endl;
		return false;
	}
	return true;
}

// Check if the shader has linked successfully
bool check_shader_link_status(const GLuint &shader_prog,
	const std::string &name)
{
	GLint success;
	glGetProgramiv(shader_prog, GL_LINK_STATUS, &success);
	if (!success) {
		// hard-coded maximum 512 bytes error message length
		GLint info_log_len = 512;
		char info_log[info_log_len];
		glGetProgramInfoLog(shader_prog, 512, nullptr, info_log);
		std::cerr << "Shader linking failed (" << name << ")\n" <<
			info_log << std::endl;
		return false;
	}
	return true;
}

}


ShaderProgram::ShaderProgram(const std::string &path_vertex_shader,
	const std::string &path_fragment_shader)
{
	std::cout << "ShaderProgram::ShaderProgram\n";
	// Load and compile the vertex shader
	GLuint vertex_shader{glCreateShader(GL_VERTEX_SHADER)};
	const std::string vertex_shader_code{read_file(path_vertex_shader)};
	const GLchar *vertex_shader_code_c{vertex_shader_code.c_str()};
	glShaderSource(vertex_shader, 1, &vertex_shader_code_c, nullptr);
	glCompileShader(vertex_shader);
	if (!check_shader_compile_status(vertex_shader, path_vertex_shader))
		throw std::runtime_error("Shader compilation failed");

	// Load and compile the fragment shader
	GLuint fragment_shader{glCreateShader(GL_FRAGMENT_SHADER)};
	const std::string fragment_shader_code{read_file(path_fragment_shader)};
	const GLchar *fragment_shader_code_c{fragment_shader_code.c_str()};
	glShaderSource(fragment_shader, 1, &fragment_shader_code_c, nullptr);
	glCompileShader(fragment_shader);
	if (!check_shader_compile_status(fragment_shader, path_fragment_shader))
		throw std::runtime_error("Shader compilation failed");

	// Link the shaders to a program
	glAttachShader(m_shader_program, vertex_shader);
	glAttachShader(m_shader_program, fragment_shader);
	glLinkProgram(m_shader_program);
	if (!check_shader_link_status(m_shader_program, path_fragment_shader))
		throw std::runtime_error("Shader linking failed");

	// Cleanup
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
}
