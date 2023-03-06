#include <string>
#include <stdexcept>
#include <iostream>

#include "texture.hpp"
#include "common.hpp"


Texture::Texture(bool enable_interpolation)
{
	glGenTextures(1, &m_id);
	bind();

	// Configure boundary behaviour, interpolation and mipmapping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	setInterpolation(enable_interpolation);
}

void Texture::loadImage(const ImageFile &img)
{
	bind();
	m_width = img.getWidth();
	m_height = img.getHeight();
	// Load the image into the bound texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, m_width, m_height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, img.getPixels());
}

void Texture::setInterpolation(bool enable_interpolation) const
{
	bind();
	if (enable_interpolation) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	} else {
		//~ glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
}
