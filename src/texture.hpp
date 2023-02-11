#pragma once

#include <GL/glew.h>


class Texture {
public:
	Texture();
	Texture(int width, int height);
	Texture(const std::string &path);
	// TODO: destructor, copy constructor, copy assignment constructor
	GLuint getId() const { return m_id; }
	int getWidth() const { return m_width; }
	int getHeight() const { return m_height; }
	void bind() const { glBindTexture(GL_TEXTURE_2D, m_id); }
	void reset(int width, int height);
	void copyFromFramebuffer(int width, int height) const;

private:
	GLuint m_id{0};
	int m_width{0};
	int m_height{0};
};

