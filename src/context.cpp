#include <algorithm>
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "context.hpp"


void Context::showHelp() const
{
	const std::string helptext{
		"General:\n"
		"	h:   Show this help\n"
		"	Mouse drag and drop:   Load an image\n"
		"\n"
		"Movement:\n"
		"	Mouse drag and drop:   Move up/down/left/right\n"
		"	arrow keys:   Move up/down/left/right\n"
		"	Shift + Mouse drag and drop:   Zoom in/out\n"
		"	.,+/-:   Zoom in/out. + may not work in the browser.\n"
		"\n"
		"Configuration:\n"
		"	i:   Toggle between nearest neighbour and linear interpolation\n"
		"	c:   Toggle histogram colour transformation\n"
		"	s + Mouse drag and drop:   Adjust the grid scaling\n"
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
				return false;
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
			case '.':
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
		} case SDL_MOUSEMOTION: {
			if (event.motion.state == SDL_BUTTON_LMASK) {
				if (m_held_keys.contains(SDLK_LSHIFT)) {
					m_renderer.getCamera().zoom(-0.005f * event.motion.yrel);
				} else if (m_held_keys.contains('s')) {
					float grid_scaling{m_renderer.getGridScaling()};
					grid_scaling *= 1.0f - 0.005f * event.motion.yrel;
					grid_scaling = std::clamp(grid_scaling, 0.1f, 3.464f);
					m_renderer.setGridScaling(grid_scaling);
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
