#pragma once

#include <algorithm>

// Designed for small, odd N
template<typename T, int N>
class Medianer {
public:
	Medianer(T starting_value)
	{
		for (auto &v : m_prev_values) {
			v = starting_value;
		}
	}

	void push(T v)
	{
		m_prev_values_i = (m_prev_values_i + 1) % N;
		m_prev_values[m_prev_values_i] = v;
	}

	// O(N log(N))
	T getMedian() const
	{
		std::array arr = m_prev_values;
		std::nth_element(arr.begin(), arr.begin() + N / 2, arr.end());
		return arr[N / 2];
	}

private:
	std::array<T, N> m_prev_values;
	int m_prev_values_i{0};
};

float running_average(float f, float v_old, float v)
{
	return f * v + (1.0f - f) * v_old;
}

// A helper class for dtime smoothing
class DtimeCalculator {
public:
	DtimeCalculator(double time):
		m_time_internal{time},
		m_time_prev{time}
	{}

	float calculateDtime(double time)
	{
		m_error_avg = running_average(0.1, m_error_avg,
			m_time_prev - m_time_internal);

		// Smooth the jittering time
		float elapsed_reported{static_cast<float>(time - m_time_prev)};
		m_time_prev = time;
		m_elapsed_reported_avg = running_average(0.01, m_elapsed_reported_avg,
			elapsed_reported);
		m_elapsed_reported_avg_median.push(m_elapsed_reported_avg);
		float elapsed_smooth{m_elapsed_reported_avg_median.getMedian()};

		// Calculate an offset to slowly correct the accumulated error over time
		float error_offset{0.1f * tanhf(m_error_avg)};


		// Determine the dtime
		float dtime{std::max(0.0f, elapsed_smooth + error_offset)};
		m_time_internal += dtime;
		return dtime;
	}

private:
	double m_time_internal{0};
	double m_time_prev{0};
	float m_elapsed_reported_avg{0.0166666666666f};
	float m_error_avg{0};
	Medianer<float, 15> m_elapsed_reported_avg_median{0.0166666666666f};
};
