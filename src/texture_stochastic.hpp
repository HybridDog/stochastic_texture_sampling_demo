#pragma once

#include <array>

#include <GL/glew.h>

#include "image_file.hpp"


// A histogram-transformed texture with LUT and a matrix for the inverse
// transformation
class TextureStochastic {
public:
	TextureStochastic(bool enable_interpolation);
	~TextureStochastic() { glDeleteTextures(1, &m_id);
		glDeleteTextures(1, &m_lut_id); }
	TextureStochastic(const TextureStochastic&) = delete;
	TextureStochastic &operator=(const TextureStochastic&) = delete;

	void loadImage(const ImageFile &img);
	int getWidth() const { return m_width; }
	int getHeight() const { return m_height; }
	void bind() const { glBindTexture(GL_TEXTURE_2D, m_id); }
	void bind_lut() const { glBindTexture(GL_TEXTURE_2D, m_lut_id); }
	std::array<float, 9> &getInverseDecorrelation() { return m_inverse_decorrelation; }
	std::array<float, 3> &getColTranslation() { return m_col_translation; }
	void setInterpolation(bool enable_interpolation) const;

private:
	GLuint m_id{0};
	GLuint m_lut_id{0};
	int m_width{0};
	int m_height{0};
	std::array<float, 9> m_inverse_decorrelation{1,0,0, 0,1,0, 0,0,1};
	std::array<float, 3> m_col_translation{0, 0, 0};
};

