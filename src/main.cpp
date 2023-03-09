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
	Context(SDL_Window *window):
		m_window{window},
		m_renderer{window}
	{
	}
	SDL_Window *m_window;
	Renderer m_renderer;
	std::set<int> m_held_keys{};
	bool m_mousedown{false};
	std::array<s32, 2> mouse_pos{0, 0};
	std::chrono::time_point<std::chrono::steady_clock> m_time{
		std::chrono::steady_clock::now()};
	float updateTime();
	bool loop();
	void showHelp() const;
};

void Context::showHelp() const
{
	const std::string helptext{
		"General:\n"
		"	h: Show this help\n"
		"\n"
		"Image-related:\n"
		"	Mouse drag and drop: Load an image\n"
		"	i: Toggle between nearest neighbour and linear interpolation\n"
		"	c: Toggle histogram colour transformation\n"
		"\n"
		"Movement:\n"
		"	Mouse drag and drop: Move up/down/left/right\n"
		"	Shift + Mouse drag and drop: Zoom in/out\n"
		"	vertical scrolling: Zoom in/out\n"
		"	Shift + vertical scrolling: Move up/down\n"
		"	Shift + horizontal scrolling: Move right/left\n"
		"	+/-: Zoom in/out. + may not work.\n"
		"	arrow keys: Move up/down/left/right\n"
	};
	std::cout << helptext;
#ifdef __EMSCRIPTEN__
	EM_ASM({
		alert(UTF8ToString($0));
	}, helptext.c_str());
#endif
}

float Context::updateTime()
{
	auto time_now{std::chrono::steady_clock::now()};
	float dtime{std::chrono::duration<float>(time_now - m_time).count()};
	m_time = time_now;
	return dtime;
}

bool Context::loop()
{
	std::array acc_dir{0.0f, 0.0f, 0.0f};
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_KEYDOWN: {
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
			case 'i':
				m_renderer.toggleInterpolation();
				break;
			case 'c':
				m_renderer.toggleColourTransformation();
				break;
			case 'h':
				showHelp();
				break;
			default:
				break;
			}
			m_held_keys.insert(event.key.keysym.sym);
			break;
		} case SDL_KEYUP: {
			m_held_keys.erase(event.key.keysym.sym);
			break;
		} case SDL_MOUSEWHEEL: {
			if (m_held_keys.contains(SDLK_LSHIFT)) {
				acc_dir[0] += event.wheel.preciseX;
				acc_dir[1] -= event.wheel.preciseY;
			} else {
				acc_dir[2] += -2.0f * event.wheel.preciseY;
			}
			break;
		} case SDL_MOUSEMOTION: {
			if (event.motion.state == SDL_BUTTON_LMASK) {
				if (m_held_keys.contains(SDLK_LSHIFT)) {
					m_renderer.getCamera().zoom(-0.01f * event.motion.yrel);
				} else {
					std::array<float, 2> move{-1.0f * event.motion.xrel,
						-1.0f * event.motion.yrel};
					m_renderer.getCamera().movePos(move);
				}
			}
			break;
		} case SDL_WINDOWEVENT: {
			if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
				m_renderer.setSize(event.window.data1, event.window.data2);
			}
			break;
		} case SDL_QUIT: {
			return false;
		} default: {
			break;
		}}
	}
	m_renderer.getCamera().update(acc_dir, updateTime());
	m_renderer.render();
	SDL_GL_SwapWindow(m_window);
	return true;
}


namespace {

#ifdef __EMSCRIPTEN__
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
	ptr_context->m_renderer.setImageLazy(filename);
}
#endif

}  // namespace


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
