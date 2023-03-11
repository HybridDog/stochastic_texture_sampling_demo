# pragma once

#include <string>
#include <vector>

#include <SDL2/SDL_image.h>

#include "common.hpp"


class ImageFile {
public:
	// Load an image from the file system
	ImageFile(const std::string &path);
	ImageFile(const ImageFile&) = delete;
	ImageFile &operator=(const ImageFile &o) = delete;
	~ImageFile() { if (m_surface) SDL_FreeSurface(m_surface); }

	auto getWidth() const { return m_surface->w; }
	auto getHeight() const { return m_surface->h; }
	// Get a pointer to the pixel values in sRGB 8-bit RGBA format
	u8 *getPixels() const { return static_cast<u8 *>(m_surface->pixels); }

private:
	SDL_Surface *m_surface{nullptr};
};

