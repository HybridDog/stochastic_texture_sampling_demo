#include <string>
#include <stdexcept>
#include <iostream>
#include <cmath>

#include "external/jacobi.h"
#include "external/linalg.h"
#include "texture_stochastic.hpp"
#include "common.hpp"
using namespace linalg::aliases;


namespace {

// Lookup table size
constexpr int LUT_WIDTH{128};

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

float3 srgb_to_linear(const float3 &col)
{
	return {srgb_to_linear(col[0]), srgb_to_linear(col[1]),
		srgb_to_linear(col[2])};
}

// column-major transformation matrix from bt.709/sRGB primaries to LMS
constexpr float3x3 RGB_TO_LMS{
	{0.4122214708f, 0.2119034982f, 0.0883024619f},
	{0.5363325363f, 0.6806995451f, 0.2817188376f},
	{0.0514459929f, 0.1073969566f, 0.6299787005f}
};

constexpr float3x3 OKLAB_MAT{
	{0.2104542553f, 1.9779984951f, 0.0259040371f},
	{0.7936177850f, -2.4285922050f, 0.7827717662f},
	{-0.0040720468f, 0.4505937099f, -0.8086757660f}
};

constexpr float3x3 OKLAB_MAT_INV{
	{1.0f, 1.0f, 1.0f},
	{0.3963377774f, -0.1055613458f, -0.0894841775f},
	{0.2158037573f, -0.0638541728f, -1.2914855480f}
};

float3 srgb_to_oklab(const float3 &col_srgb)
{
	float3 col{srgb_to_linear(col_srgb)};
	col = mul(RGB_TO_LMS, col);
	col = {std::pow(col[0], 1.0f / 3.0f), std::pow(col[1], 1.0f / 3.0f),
		std::pow(col[2], 1.0f / 3.0f)};
	return mul(OKLAB_MAT, col);
}

// Calculate a rotation matrix to decorrelate the colour channels
float3x3 get_correlation_matrix(const std::vector<float4> &values)
{
	// Calculate the covariance matrix of the colours weighted with the
	// transparency
	float w_acc{0};
	float3 mean{0, 0, 0};
	for (const auto &v : values) {
		const float3 &rgb{reinterpret_cast<const float3 &>(v)};
		float weight{v[3]};
		mean += weight * rgb;
		w_acc += weight;
	}
	mean /= w_acc;
	float3x3 cov{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
	for (const auto &v : values) {
		const float3 &rgb{reinterpret_cast<const float3 &>(v)};
		float weight{v[3]};
		float3 rgb_off{rgb - mean};
		// m is rgb_off * transpose(rgb_off)
		float3x3 m{rgb_off[0] * rgb_off, rgb_off[1] * rgb_off,
			rgb_off[2] * rgb_off};
		cov = cov + weight * m;
	}
	// For the eigenvectors, we can ignore this factor in the covariance matrix
	// cov /= w_acc - 1.0f;

	// Calculate the eigenvectors of the covariance matrix
	double cov_tmp[3][3]{
		{cov[0][0], cov[0][1], cov[0][2]},
		{cov[1][0], cov[1][1], cov[1][2]},
		{cov[2][0], cov[2][1], cov[2][2]}
	};
	double eigenvecs_tmp[3][3];
	double eigenvals_tmp[3];
	ComputeEigenValuesAndVectors(cov_tmp, eigenvecs_tmp, eigenvals_tmp);
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
	float3x3 evs{get_correlation_matrix(values)};
	for (auto &v : values) {
		const float3 &rgb{reinterpret_cast<const float3 &>(v)};
		float3 rotated{mul(evs, rgb)};
		v[0] = rotated[0];
		v[1] = rotated[1];
		v[2] = rotated[2];
	}
	float3x3 inv{mul(OKLAB_MAT_INV, transpose(evs))};
	// linalg.h and GLSL uniform matrices are column-major
	inv_transform = {inv[0][0], inv[0][1], inv[0][2],
		inv[1][0], inv[1][1], inv[1][2],
		inv[2][0], inv[2][1], inv[2][2]};
}

constexpr float GAUSSIAN_AVERAGE{0.5f};
constexpr float GAUSSIAN_STD{0.16666f};

std::vector<float4> histogram_transformation(std::vector<float4> &values)
{
	std::vector<float4> lut;
	lut.resize(LUT_WIDTH);
	// Initialize permutation arrays for sorting
	std::vector<int> perm_visible, perm_all;
	perm_visible.reserve(values.size());
	perm_all.reserve(values.size());
	for (size_t i{0}; i < values.size(); ++i) {
		// Skip fully transparent pixels
		if (values[i][3] > 0) {
			perm_visible.push_back(i);
		}
		perm_all.push_back(i);
	}
	if (perm_visible.size() < 1)
		// No visible pixels -> do nothing
		return lut;
	for (int c = 0; c < 4; ++c) {
		// For the colour channels, exclude fully transparent pixels
		std::vector<int> &perm{c < 3 ? perm_visible : perm_all};

		// Sort perm such that values[perm[n]][c] < values[perm[n+1]][c]
		// for all n, 0 <= n < values.size()
		std::sort(perm.begin(), perm.end(), [&values, &c](int a, int b) {
			return values[a][c] < values[b][c]; });

		// Create the LUT for the inverse histogram transformation
		for (int k{0}; k < LUT_WIDTH; ++k) {
			float U{CDF((k + 0.5f) / LUT_WIDTH, GAUSSIAN_AVERAGE, GAUSSIAN_STD)};
			int idx_perm{static_cast<int>(floor(U * perm.size()))};
			lut[k][c] = values[perm[idx_perm]][c];
		}

		// Do the histogram transformation
		for (size_t k{0}; k < perm.size(); ++k) {
			float U = (k + 0.5f) / perm.size();
			values[perm[k]][c] = invCDF(U, GAUSSIAN_AVERAGE, GAUSSIAN_STD);
		}

	}
	return lut;
}

} // namespace


TextureStochastic::TextureStochastic(bool enable_interpolation)
{
	// Create and configure the textures
	glGenTextures(1, &m_id);
	bind();
	// Configure boundary behaviour, interpolation and mipmapping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	setInterpolation(enable_interpolation);

	// Create and configure the LUT texture
	glGenTextures(1, &m_lut_id);
	bind_lut();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void TextureStochastic::loadImage(const ImageFile &img)
{
	m_width = img.getWidth();
	m_height = img.getHeight();

	// Load the image and convert it to floats in a perceptual colourspace
	std::vector<float4> pixels;
	pixels.reserve(m_width * m_height);
	byte4 *pixels_raw{reinterpret_cast<byte4 *>(img.getPixels())};
	for (int i = 0; i < m_width * m_height; ++i) {
		float4 rgba{pixels_raw[i] / 255.0f};
		float3 col{rgba[0], rgba[1], rgba[2]};
		col = srgb_to_oklab(col);
		rgba[0] = col[0];
		rgba[1] = col[1];
		rgba[2] = col[2];
		pixels.emplace_back(rgba);
	}

	// Decorrelate the colours and perform the histogram transformations
	decorrelate_colours(pixels, m_inverse_decorrelation);
	std::vector<float4> lut{histogram_transformation(pixels)};

	// Upload to GPU
	bind();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0,
		GL_RGBA, GL_FLOAT, pixels.data());
	bind_lut();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, LUT_WIDTH, 1, 0,
		GL_RGBA, GL_FLOAT, lut.data());
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
