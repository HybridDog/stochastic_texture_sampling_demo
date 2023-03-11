#include <fstream>
#include <iostream>

#include <SDL2/SDL.h>
#include <common.hpp>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "context.hpp"
#ifdef __EMSCRIPTEN__
#include "emscripten-browser-file/emscripten_browser_file.h"
#endif


#ifdef __EMSCRIPTEN__
namespace {

void emscripten_loop(void *arg)
{
	Context *c{static_cast<Context *>(arg)};
	c->loop();
}

Context *ptr_context;
void emscripten_on_file_upload(std::string const &filename,
	std::string const &mime_type, std::string_view buffer, void *arg)
{
	// FIXME: Passing the context to upload apparently does not work correctly
	//~ ptr_context = static_cast<Context *>(arg);
	if (buffer.size() == 0) {
		std::cerr << "Empty file was uploaded; filename: " << filename <<
			", mime_type: " << mime_type << std::endl;
		return;
	}
	// Save the image file and let the browser decode it
	std::ofstream outfile(filename, std::ios::binary);
	if (!outfile.is_open())
		throw std::runtime_error("Could not open " + filename + " for writing");
	outfile.write(buffer.data(), buffer.size());
	outfile.flush();
	ptr_context->getRenderer().setImageLazy(filename);
}

}  // namespace
#endif


int main()
{
	SDL_Window *window{SDL_CreateWindow(
		"Stochastic Texture Samping Demonstration", 0, 0, 800, 600,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE)};
	SDL_GL_CreateContext(window);
	GLenum err = glewInit();
	if (err != GLEW_OK)
		return 1;
#ifdef __EMSCRIPTEN__
	Context *c_ptr{new Context{window}};
	ptr_context = c_ptr;
	// The accepted files argument of upload is currently ignored since this
	// function is changed
	emscripten_browser_file::upload(".png,.jpg,.jpeg",
		emscripten_on_file_upload, c_ptr);
	emscripten_set_main_loop_arg(emscripten_loop, c_ptr, -1, 1);
	delete c_ptr;
#else
	Context c{window};
	for(;;) {
		if (!c.loop())
			break;
	}
#endif
	return 0;
}
