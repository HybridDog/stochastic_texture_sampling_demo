#include <string>
#include <stdexcept>
#include <iostream>

#include "texture_stochastic.hpp"
#include "image_file.hpp"
#include "common.hpp"
#include "jacobi.h"
#include "linalg.h"
using namespace linalg::aliases;


namespace {

float srgb_to_linear(float v)
{
	if (v <= 0.04045f)
		return v / 12.92f;
	else
		return powf((v + 0.055f) / (1.055f), 2.4f);
}

float3x3 get_decorrelation_matrix(const std::vector<float4> &values)
{
	// Calculate the covariance matrix
	float3 rgb_mean{0, 0, 0};
	float3 rr_gg_bb_mean{0, 0, 0};
	float3 rg_rb_gb_mean{0, 0, 0};
	for (const auto &v : values) {
		const float3 &rgb{reinterpret_cast<const float3 &>(v)};
		rgb_mean += rgb;
		rr_gg_bb_mean += rgb * rgb;
		float3 rg_rb_gb{rgb[0] * rgb[1], rgb[0] * rgb[2], rgb[1] * rgb[2]};
		rg_rb_gb_mean += rg_rb_gb;
	}
	rgb_mean *= 1.0f / values.size();
	rr_gg_bb_mean *= 1.0f / values.size();
	rg_rb_gb_mean *= 1.0f / values.size();
	float rm{rgb_mean[0]};
	float gm{rgb_mean[1]};
	float bm{rgb_mean[2]};
	float rgm{rg_rb_gb_mean[0]};
	float rbm{rg_rb_gb_mean[1]};
	float gbm{rg_rb_gb_mean[2]};
	double3x3 cov{
		{rr_gg_bb_mean[0] - rm * rm, rgm - rm * gm, rbm - rm * bm},
		{rgm - rm * gm, rr_gg_bb_mean[1] - gm * gm, gbm - gm * bm},
		{rbm - rm * bm, gbm - gm * bm, rr_gg_bb_mean[3] - bm * bm},
	};

	// Calculate the eigenvectors of the covariance matrix
	double cov_tmp[3][3]{
		{cov[0][0], cov[0][1], cov[0][2]},
		{cov[1][0], cov[1][1], cov[1][2]},
		{cov[2][0], cov[2][1], cov[2][2]}
	};
	double eigenvecs_tmp[3][3];
	double eigenvals_tmp[3];
	ComputeEigenValuesAndVectors(cov_tmp, eigenvecs_tmp, eigenvals_tmp);
	// TODO: do I need to transpose this or not? :(
	double3x3 m{
		{eigenvecs_tmp[0][0],
			eigenvecs_tmp[0][1],
			eigenvecs_tmp[0][2]},
		{eigenvecs_tmp[1][0],
			eigenvecs_tmp[1][1],
			eigenvecs_tmp[1][2]},
		{eigenvecs_tmp[2][0],
			eigenvecs_tmp[2][1],
			eigenvecs_tmp[2][2]}};
	float3x3 m_f{m};
	return m_f;
}

void decorrelate_colours(std::vector<float4> &values,
	std::array<float, 9> &inv_transform)
{
	float3x3 evs{get_decorrelation_matrix(values)};
	for (auto &v : values) {
		const float3 &rgb{reinterpret_cast<const float3 &>(v)};
		float3 rotated{mul(evs, rgb)};
		v[0] = rotated[0];
		v[1] = rotated[1];
		v[2] = rotated[2];
	}
	// TODO: do I need to transpose this or not? Matrix inversion is missing
	inv_transform = {evs[0][0], evs[1][0], evs[2][0],
		evs[0][1], evs[1][1], evs[2][1],
		evs[0][2], evs[1][2], evs[2][2]};
}

void histogram_transformation(std::vector<float4> &values)
{
	// TODO:
	// * transpose values into 4 float std::vectors
	// * For each of them:
	//   * sort it
	//   * apply forward LUT and calculate backward LUT
	// * transpose the 4 float vectors back into values (overriding values)
	// * return the backward LUTs
}

} // namespace


TextureStochastic::TextureStochastic(const std::string &path,
	bool enable_interpolation)
{
	glGenTextures(1, &m_id);
	bind();

	// Load the image and convert it to floats in linear colourspace
	ImageFile img{path};
	m_width = img.getWidth();
	m_height = img.getHeight();
	std::vector<float4> pixels;
	pixels.reserve(m_width * m_height);
	byte4 *pixels_raw{reinterpret_cast<byte4 *>(img.getPixels())};
	for (int i = 0; i < m_width * m_height; ++i) {
		float4 rgba{pixels_raw[i] / 255.0f};
		rgba[0] = srgb_to_linear(rgba[0]);
		rgba[1] = srgb_to_linear(rgba[1]);
		rgba[2] = srgb_to_linear(rgba[2]);
		pixels.emplace_back(rgba);
	}

	// Decorrelate the colours
	std::array<float, 9> inv_transform;
	decorrelate_colours(pixels, inv_transform);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0,
		GL_RGBA, GL_FLOAT, pixels.data());

	// Configure boundary behaviour, interpolation and mipmapping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	setInterpolation(enable_interpolation);

	//~ glGenerateMipmap(GL_TEXTURE_2D);
}




void TextureStochastic::setInterpolation(bool enable_interpolation) const
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
