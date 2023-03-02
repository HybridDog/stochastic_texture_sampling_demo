#include <string>
#include <stdexcept>
#include <iostream>
#include <cmath>

#include "texture_stochastic.hpp"
#include "image_file.hpp"
#include "common.hpp"
#include "jacobi.h"
#include "linalg.h"
using namespace linalg::aliases;

// copy-pasted from the original demo
#define GAUSSIAN_AVERAGE 0.5f // Expectation of the Gaussian distribution
#define GAUSSIAN_STD 0.16666f // Std of the Gaussian distribution
#define LUT_WIDTH 128 // Size of the look-up table


namespace {

// math functions copy-pasted from the official demo

float Erf(float x)
{
	// Save the sign of x
	int sign = 1;
	if (x < 0)
		sign = -1;
	x = abs(x);

	// A&S formula 7.1.26
	float t = 1.0f / (1.0f + 0.3275911f * x);
	float y = 1.0f - (((((1.061405429f * t + -1.453152027f) * t) + 1.421413741f)
		* t + -0.284496736f) * t + 0.254829592f) * t * exp(-x * x);

	return sign * y;
}

float ErfInv(float x)
{
	float w, p;
	w = -log((1.0f - x) * (1.0f + x));
	if (w < 5.000000f)
	{
		w = w - 2.500000f;
		p = 2.81022636e-08f;
		p = 3.43273939e-07f + p * w;
		p = -3.5233877e-06f + p * w;
		p = -4.39150654e-06f + p * w;
		p = 0.00021858087f + p * w;
		p = -0.00125372503f + p * w;
		p = -0.00417768164f + p * w;
		p = 0.246640727f + p * w;
		p = 1.50140941f + p * w;
	}
	else
	{
		w = sqrt(w) - 3.000000f;
		p = -0.000200214257f;
		p = 0.000100950558f + p * w;
		p = 0.00134934322f + p * w;
		p = -0.00367342844f + p * w;
		p = 0.00573950773f + p * w;
		p = -0.0076224613f + p * w;
		p = 0.00943887047f + p * w;
		p = 1.00167406f + p * w;
		p = 2.83297682f + p * w;
	}
	return p * x;
}

float CDF(float x, float mu, float sigma)
{
	float U = 0.5f * (1 + Erf((x-mu)/(sigma*sqrtf(2.0f))));
	return U;
}

float invCDF(float U, float mu, float sigma)
{
	float x = sigma*sqrtf(2.0f) * ErfInv(2.0f*U-1.0f) + mu;
	return x;
}


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
	// TODO: 1/(values.size()-1) statt 1/values.size() bei Varianz
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

std::vector<float4> histogram_transformation(std::vector<float4> &values)
{
	std::vector<float4> lut;
	lut.resize(LUT_WIDTH);
	const size_t num_pixels{values.size()};
	// Determine a permutation for sorting
	std::vector<int> perm;
	perm.reserve(num_pixels);
	for (size_t i{0}; i < num_pixels; ++i) {
		perm.push_back(i);
	}
	for (int i = 0; i < 4; ++i) {
		// Sort perm such that values[perm[n]][i] < values[perm[n+1]][i]
		// for all n, 0 <= n < values.size()
		std::sort(perm.begin(), perm.end(), [&values, &i](int a, int b) {
			return values[a][i] < values[b][i]; });

		// Create the LUT for the inverse histogram transformation
		for (int k{0}; k < LUT_WIDTH; ++k) {
			float U{CDF((k + 0.5f) / LUT_WIDTH, GAUSSIAN_AVERAGE, GAUSSIAN_STD)};
			lut[k][i] = values[perm[static_cast<int>(floor(U * num_pixels))]][i];
			//~ lut[k][i] = U;
			//~ lut[k][i] = (float)k/LUT_WIDTH;
		}

		// Do the histogram transformation
		for (size_t k{0}; k < num_pixels; ++k) {
			float U = (k + 0.5f) / num_pixels;
			//~ values[perm[k]][i] = invCDF(U, GAUSSIAN_AVERAGE, GAUSSIAN_STD);
		}

	}
	return lut;
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

	// Decorrelate the colours and perform the histogram transformations
	decorrelate_colours(pixels, m_inverse_decorrelation);
	std::vector<float4> lut{histogram_transformation(pixels)};

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0,
		GL_RGBA, GL_FLOAT, pixels.data());

	// Configure boundary behaviour, interpolation and mipmapping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	setInterpolation(enable_interpolation);

	// Copy the LUT to GPU and configure it
	glGenTextures(1, &m_lut_id);
	bind_lut();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0,
		GL_RGBA, GL_FLOAT, lut.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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
