#include <stdio.h>
#include <assert.h>
#include <iostream>
#include <set>

#include <SDL2/SDL.h>
#include <common.hpp>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "renderer.hpp"
#include "dtime_calculator.hpp"

struct Context {
	SDL_Window *window;
	Renderer renderer{800, 600};
	std::set<int> held_keys{};
};

bool loop(Context &c)
{
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
	//~ float dtime{timer.calculateDtime(SDL_GetTicks64() * 0.001)};
	//~ renderer.getCamera().update(acc_dir, dtime);
	c.renderer.getCamera().update(acc_dir, 0.017);
	c.renderer.render();
	SDL_GL_SwapWindow(c.window);
	return true;
}

void emscripten_loop(void *arg)
{
	Context *c{static_cast<Context *>(arg)};
	loop(*c);
}

int main()
{
	SDL_Window *window{SDL_CreateWindow("mein prog", 0, 0, 800, 600,
		SDL_WINDOW_OPENGL)};
	assert(window);
	//~ SDL_GLContext context{SDL_GL_CreateContext(window)};
	SDL_GL_CreateContext(window);
	GLenum err = glewInit();
	if (err != GLEW_OK)
		return 1;
	Context c{window};
	// DtimeCalculator timer{SDL_GetTicks64() * 0.001};
#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop_arg(emscripten_loop, &c, -1, 1);
#else
	for(;;) {
		if (!loop(c))
			break;
	}
#endif
	return 0;
}
