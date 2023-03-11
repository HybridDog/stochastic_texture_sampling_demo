#pragma once

#include <array>

class Camera {
public:
	// Move the camera with acceleration and velocity for a given time.
	// The acceleration is rescaled internally.
	void update(std::array<float, 3> &acc_dir, float dtime);
	// Get the current camera position, except Z distance
	std::array<float, 2> getPos() const { return {m_pos[0], -m_pos[1]}; }
	// Move the camera position along X or Y. The movement is scaled with the
	// Z distance.
	void movePos(std::array<float, 2> &move) {
		m_pos[0] += move[0] * m_pos[2], m_pos[1] += move[1] * m_pos[2]; }
	// Move closer or further away in Z direction
	void zoom(float z) { m_pos[2] = m_pos[2] + z * m_pos[2];
		if (m_pos[2] < 0) m_pos[2] = 0.000001f; }
	// Get the inverse Z distance
	float getZoom() const { return 1.0f / m_pos[2]; }

private:
	std::array<float, 3> m_vel{0, 0, 0};
	std::array<float, 3> m_pos{0, 0, 0.01f};
};

