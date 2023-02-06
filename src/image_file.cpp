#include "image_file.hpp"

ImageFile::ImageFile(const std::string &path):
	m_surface{IMG_Load(path.c_str())}
{
	// Convert the image to ensure that it is in RGB format
	SDL_Surface *convert{SDL_CreateRGBSurface(0, m_surface->w, m_surface->h, 24,
		0xff000000, 0x00ff0000, 0x0000ff00, 0x00000000)};
	SDL_SetSurfaceBlendMode(m_surface, SDL_BLENDMODE_NONE);
	SDL_BlitSurface(m_surface, nullptr, convert, nullptr);
	SDL_FreeSurface(m_surface);
	m_surface = convert;
}
