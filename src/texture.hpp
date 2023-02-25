#pragma once

#include <GL/glew.h>


class Texture {
public:
	Texture();
	Texture(const std::string &path, bool enable_interpolation);
	// TODO: destructor, copy constructor, copy assignment constructor
	GLuint getId() const { return m_id; }
	int getWidth() const { return m_width; }
	int getHeight() const { return m_height; }
	void bind() const { glBindTexture(GL_TEXTURE_2D, m_id); }
	void setInterpolation(bool enable_interpolation) const;

private:
	GLuint m_id{0};
	int m_width{0};
	int m_height{0};
};

