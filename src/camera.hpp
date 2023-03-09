#pragma once

#include <array>

class Camera {
public:
	void update(std::array<float, 3> &acc_dir, float dtime);
	std::array<float, 2> getPos() const { return {m_pos[0], -m_pos[1]}; }
	void movePos(std::array<float, 2> &move) {
		m_pos[0] += move[0] * m_pos[2], m_pos[1] += move[1] * m_pos[2]; }
	void zoom(float z) { m_pos[2] = m_pos[2] + z * m_pos[2];
		if (m_pos[2] < 0) m_pos[2] = 0.000001; }
	float getZoom() const { return 1.0f / m_pos[2]; }

private:
	std::array<float, 3> m_vel{0, 0, 0};
	std::array<float, 3> m_pos{0, 0, 0.01};
};

