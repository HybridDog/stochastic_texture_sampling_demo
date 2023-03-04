# include <iostream>

#include "image_file.hpp"

namespace {

// Process a SDL surface to ensure it is in a RGBA format
void convert_surface(SDL_Surface *surface)
{
	// Convert the image to ensure that it is in RGBA format
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	u32 rmask = 0xff000000;
	u32 gmask = 0x00ff0000;
	u32 bmask = 0x0000ff00;
	u32 amask = 0x000000ff;
#else
	u32 rmask = 0x000000ff;
	u32 gmask = 0x0000ff00;
	u32 bmask = 0x00ff0000;
	u32 amask = 0xff000000;
#endif
	SDL_Surface *convert{SDL_CreateRGBSurface(0, surface->w, surface->h, 32,
		rmask, gmask, bmask, amask)};
	if (convert == nullptr)
		throw std::runtime_error("SDL_CreateRGBSurface has failed");
	SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);
	SDL_BlitSurface(surface, nullptr, convert, nullptr);
	SDL_FreeSurface(surface);
	surface = convert;
}

}  // namespace

ImageFile::ImageFile(const std::string &path):
	m_surface{IMG_Load(path.c_str())}
{
	if (m_surface == nullptr)
		throw std::runtime_error("Could not load image " + path);
	convert_surface(m_surface);
}

ImageFile::ImageFile(const std::vector<u8> &data)
{
	std::cout << data.size() << std::endl;
	SDL_RWops *ops{SDL_RWFromConstMem(data.data(), data.size())};
	m_surface = IMG_Load_RW(ops, 1);
	if (m_surface == nullptr)
		throw std::runtime_error("Could not load image");
	convert_surface(m_surface);
}
