# include <iostream>

#include "image_file.hpp"

ImageFile::ImageFile(const std::string &path):
	m_surface{IMG_Load(path.c_str())}
{
	if (m_surface == nullptr)
		throw std::runtime_error("Could not load image " + path);
	// Convert the image to ensure that it is in RGB format
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
	SDL_Surface *convert{SDL_CreateRGBSurface(0, m_surface->w, m_surface->h, 32,
		rmask, gmask, bmask, amask)};
	if (convert == nullptr)
		throw std::runtime_error("SDL_CreateRGBSurface has failed");
	SDL_SetSurfaceBlendMode(m_surface, SDL_BLENDMODE_NONE);
	SDL_BlitSurface(m_surface, nullptr, convert, nullptr);
	SDL_FreeSurface(m_surface);
	m_surface = convert;
}
