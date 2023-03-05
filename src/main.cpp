#include <stdio.h>
#include <assert.h>
#include <iostream>
#include <set>
#include <chrono>

#include <SDL2/SDL.h>
#include <common.hpp>
#ifdef __EMSCRIPTEN__
#include <fstream>
#include <emscripten.h>
#endif

#include "renderer.hpp"
#include "image_file.hpp"
#ifdef __EMSCRIPTEN__
#include "emscripten-browser-file/emscripten_browser_file.h"
#endif


struct Context {
	SDL_Window *window;
	Renderer renderer{1500, 800};
	std::set<int> held_keys{};
	std::string image_file_to_load{""};
	bool should_load_image_file{false};
	std::chrono::time_point<std::chrono::steady_clock> m_time{
		std::chrono::steady_clock::now()};
	float updateTime();
};

float Context::updateTime()
{
	auto time_now{std::chrono::steady_clock::now()};
	float dtime{std::chrono::duration<float>(time_now - m_time).count()};
	m_time = time_now;
	return dtime;
}

bool loop(Context &c)
{
	if (c.should_load_image_file) {
		c.renderer.setImage(ImageFile{c.image_file_to_load});
		// FIXME: delete the file here to free memory
		c.image_file_to_load = "";
		c.should_load_image_file = false;
	}
	std::array acc_dir{0.0f, 0.0f, 0.0f};
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_KEYDOWN) {
			switch (event.key.keysym.sym) {
			case SDLK_ESCAPE:
				return 0;
			case SDLK_LEFT:
				acc_dir[0] -= 1.0f;
				break;
			case SDLK_RIGHT:
				acc_dir[0] += 1.0f;
				break;
			case SDLK_UP:
				acc_dir[1] -= 1.0f;
				break;
			case SDLK_DOWN:
				acc_dir[1] += 1.0f;
				break;
			case '+':
				// + does not work in firefox
				acc_dir[2] -= 1.0f;
				break;
			case '-':
				acc_dir[2] += 1.0f;
				break;
			default:
				break;
			}
			c.held_keys.insert(event.key.keysym.sym);
		} else if (event.type == SDL_KEYUP) {
			c.held_keys.erase(event.key.keysym.sym);
		} else if (event.type == SDL_MOUSEWHEEL) {
			if (c.held_keys.contains(SDLK_LSHIFT)) {
				acc_dir[2] += -2.0f * event.wheel.preciseY;
			} else {
				acc_dir[0] += event.wheel.preciseX;
				acc_dir[1] += event.wheel.preciseY;
			}
		} else if (event.type == SDL_QUIT) {
			return false;
		}
	}
	c.renderer.getCamera().update(acc_dir, c.updateTime());
	c.renderer.render();
	SDL_GL_SwapWindow(c.window);
	return true;
}

#ifdef __EMSCRIPTEN__
void emscripten_loop(void *arg)
{
	Context *c{static_cast<Context *>(arg)};
	loop(*c);
}

Context *ptr_context;
void on_file_decoded(const char *)
{
	ptr_context->should_load_image_file = true;
}
void on_file_decoded_error(const char *)
{
	std::cerr << "Could not decode image " <<
		ptr_context->image_file_to_load << "\n";
	// FIXME: delete the file here to free memory
}

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
	ptr_context->image_file_to_load = filename;
	emscripten_run_preload_plugins(filename.c_str(), on_file_decoded,
		on_file_decoded_error);
}
#endif

int main()
{
	SDL_Window *window{SDL_CreateWindow("mein prog", 0, 0, 1500, 800,
		SDL_WINDOW_OPENGL)};
	assert(window);
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
		if (!loop(c))
			break;
	}
#endif
	return 0;
}
