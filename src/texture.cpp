#include <string>
#include <stdexcept>
#include <iostream>

#include "texture.hpp"
#include "image_file.hpp"
#include "common.hpp"


Texture::Texture()
{
	glGenTextures(1, &m_id);
	bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//~ glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//~ glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

Texture::Texture(int width, int height)
{
	Texture();
	m_width = width;
	m_height = height;
	// Use GL_RGB16F instead of GL_RGB so that fragment shader output is not
	// clamped to [0, 1]
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB,
		GL_UNSIGNED_BYTE, nullptr);
}


namespace {

// Load an image into the currently bound OpenGL Texture
void load_image_file(const std::string &path, int &width, int &height)
{
	ImageFile img{path};
	width = img.getWidth();
	height = img.getHeight();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, width, height, 0, GL_RGB,
		GL_UNSIGNED_BYTE, img.getPixels());
}

}  // namespace


Texture::Texture(const std::string &path)
{
	glGenTextures(1, &m_id);
	bind();
	load_image_file(path, m_width, m_height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture::reset(int width, int height)
{
	m_width = width;
	m_height = height;
	bind();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB,
		GL_UNSIGNED_BYTE, nullptr);
}

