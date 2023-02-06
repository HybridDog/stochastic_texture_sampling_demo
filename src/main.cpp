#include <stdio.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
//~ #include <GL/gl.h>
#include <common.hpp>


int main()
{
	SDL_Window *window{SDL_CreateWindow("mein prog", 0, 0, 800, 600,
		SDL_WINDOW_OPENGL)};
	assert(window);
	SDL_GLContext context{SDL_GL_CreateContext(window)};
	for(;;) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
					return 0;
				default:
					break;
				}
			} else if (event.type == SDL_QUIT) {
				return 0;
			}
		}
		glViewport(0, 0, 800, 600);
		glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		SDL_GL_SwapWindow(window);
	}
	return 0;
}
