#pragma once

#include <GL/glew.h>

#include "image_file.hpp"


// A texture for simple rendering
class Texture {
public:
	Texture(bool enable_interpolation);
	~Texture() { glDeleteTextures(1, &m_id); }
	Texture(const Texture&) = delete;
	Texture &operator=(const Texture&) = delete;

	void loadImage(const ImageFile &img);
	int getWidth() const { return m_width; }
	int getHeight() const { return m_height; }
	void bind() const { glBindTexture(GL_TEXTURE_2D, m_id); }
	void setInterpolation(bool enable_interpolation) const;

private:
	GLuint m_id{0};
	int m_width{0};
	int m_height{0};
};

