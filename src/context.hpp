#pragma once

#include <set>
#include <chrono>

#include "renderer.hpp"


// Helper class to maintain the global state of the application
class Context {
public:
	Context(SDL_Window *window):
		m_window{window},
		m_renderer{window}
	{
	}
	// Function which should be executed each frame
	bool loop();
	Renderer &getRenderer() { return m_renderer; }

private:
	// Calculate a time difference to the previous time this function was called
	float updateTime();
	// Show a help text
	void showHelp() const;

	SDL_Window *m_window;
	Renderer m_renderer;
	std::set<int> m_held_keys{};
	std::chrono::time_point<std::chrono::steady_clock> m_time{
		std::chrono::steady_clock::now()};
};
