// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    zcross.cpp

    Zero-crossing comparator for line-level audio

*********************************************************************/

#include "emu.h"
#include "zcross.h"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <vector>


namespace {

constexpr int SINC_PHASES = 512;

// Hann-windowed sinc, TAPS samples wide, sampled at 1/SINC_PHASES sample steps
float const *sinc_table()
{
	static std::vector<float> const table = [] ()
	{
		int const half = zero_crossing_comparator::TAPS / 2;
		std::vector<float> result(zero_crossing_comparator::TAPS * SINC_PHASES + 1);
		for (int i = 0; i < int(result.size()); i++)
		{
			constexpr double PI = std::numbers::pi;
			double const d = double(i) / SINC_PHASES - half;
			double const sinc = (d == 0.0) ? 1.0 : std::sin(PI * d) / (PI * d);
			result[i] = float(sinc * (0.5 + 0.5 * std::cos(PI * d / half)));
		}
		return result;
	}();

	return table.data();
}

} // anonymous namespace

zero_crossing_comparator::zero_crossing_comparator(s16 hysteresis) :
	m_hysteresis(hysteresis),
	m_state(true)
{
	std::fill(std::begin(m_history), std::end(m_history), 0);
}

bool zero_crossing_comparator::update(s16 sample, double &position)
{
	std::copy(std::begin(m_history) + 1, std::end(m_history), std::begin(m_history));
	m_history[TAPS - 1] = sample;

	s16 const last = m_history[TAPS / 2 - 1];
	s16 const current = m_history[TAPS / 2];

	if (m_state ? (current >= -m_hysteresis) : (current <= m_hysteresis))
		return false;

	m_state = !m_state;
	position = 0.0;

	if ((last < 0) != (current < 0))
	{
		float const *const table = sinc_table();
		auto const level = [this, table] (int phase)
		{
			float result = 0.0f;
			for (int i = 0; i < TAPS; i++)
				result += m_history[i] * table[(TAPS - 1 - i) * SINC_PHASES + phase];
			return result;
		};

		int lo = 0, hi = SINC_PHASES;
		bool const lo_negative = last < 0;
		while (hi - lo > 1)
		{
			int const mid = (lo + hi) / 2;
			if ((level(mid) < 0.0f) == lo_negative)
				lo = mid;
			else
				hi = mid;
		}

		position = double(lo + hi) / (2 * SINC_PHASES);
	}

	return true;
}

void zero_crossing_comparator::register_save(device_t &device, int index)
{
	device.save_item(NAME(m_state), index);
	device.save_item(NAME(m_history), index);
}
