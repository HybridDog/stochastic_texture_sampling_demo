#include <stdio.h>
#include <assert.h>
#include <iostream>

#include <SDL2/SDL.h>
#include <common.hpp>

#include "renderer.hpp"
#include "dtime_calculator.hpp"


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
	Renderer renderer{800, 600};
	DtimeCalculator timer{SDL_GetTicks64() * 0.001};
	for(;;) {
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
					acc_dir[2] -= 1.0f;
					break;
				case '-':
					acc_dir[2] += 1.0f;
					break;
				default:
					break;
				}
			} else if (event.type == SDL_QUIT) {
				return 0;
			}
		}
		float dtime{timer.calculateDtime(SDL_GetTicks64() * 0.001)};
		renderer.getCamera().update(acc_dir, dtime);
		renderer.render();
		SDL_GL_SwapWindow(window);
	}
	return 0;
}
