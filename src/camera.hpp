#pragma once

#include <array>

class Camera {
public:
	void update(std::array<float, 3> &acc_dir, float dtime);
	std::array<float, 2> getPos() const { return {m_pos[0], m_pos[1]}; };
	float getZoom() const { return 1.0f / m_pos[2]; };

private:
	std::array<float, 3> m_vel{0, 0, 0};
	std::array<float, 3> m_pos{0, 0, 0.01};
};

