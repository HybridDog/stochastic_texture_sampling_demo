#include <cmath>
#include <iostream>

#include "camera.hpp"

void Camera::update(std::array<float, 3> &acc_dir, float dtime)
{
	float acc_len = sqrtf(acc_dir[0] * acc_dir[0] + acc_dir[1] * acc_dir[1]
		+ acc_dir[2] * acc_dir[2]);
	std::array<float, 3> acc{0, 0, 0};
	if (acc_len > 0) {
		for (int i = 0; i < 3; ++i) {
			acc[i] = acc_dir[i] / acc_len;
		}
		// Accelerate faster if we are further away, slower in z direction
		acc[0] *= 20000.0f * m_pos[2];
		acc[1] *= 20000.0f * m_pos[2];
	}
	// Formally incorrect; move the camera
	for (int i = 0; i < 3; ++i) {
		m_pos[i] += acc[i] * dtime * dtime + m_vel[i] * dtime;
		m_vel[i] += acc[i] * dtime;
		// Gradually slow down
		m_vel[i] *= powf(0.01f, dtime);
	}

	// Bounce off
	if (m_pos[2] <= 0) {
		m_pos[2] *= -1;
		m_vel[2] *= -1;
	}
}
