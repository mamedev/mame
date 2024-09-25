// license:BSD-3-Clause
// copyright-holders:SomeRandomGuyIdk
/**********************************************************************

    Electromechanical reels for slot machines

**********************************************************************/

#ifndef MAME_MACHINE_EM_REEL_H
#define MAME_MACHINE_EM_REEL_H

#pragma once

#include <iterator>
#include <utility>
#include <vector>


class em_reel_device : public device_t
{
public:
	enum class dir : uint8_t
	{
		FORWARD = 0, // Steps count up
		REVERSE // Steps count down
	};

	em_reel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <typename T>
	em_reel_device(
			const machine_config &mconfig, const char *tag, device_t *owner,
			uint16_t steps, T &&detents, attotime period, dir direction = dir::REVERSE) :
		em_reel_device(mconfig, tag, owner, 0)
	{
		set_max_pos(steps);
		set_detents(std::forward<T>(detents));
		set_rotation_period(period);
		set_direction(direction);
	}

	// Movement state callback, used for playing samples
	auto state_changed_callback() { return m_state_cb.bind(); }

	// Set period for one full rotation
	void set_rotation_period(attotime period) { m_step_period = period / (m_max_pos + 1); }

	// Get the reel's current step position
	uint16_t get_pos() const { return m_pos; }
	// Start and stop the reel
	void set_state(uint8_t state);
	// Set the direction the reel moves in
	void set_direction(dir direction) { m_direction = direction; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_validity_check(validity_checker &valid) const override;

private:
	enum class reel_state : uint8_t
	{
		STOPPED = 0,
		SPINNING,
		STOPPING
	};

	TIMER_CALLBACK_MEMBER(move);

	void set_max_pos(uint16_t steps) { m_max_pos = steps - 1; }
	template <typename T> void set_detents(T &&detents) { m_detents.assign(std::begin(detents), std::end(detents)); }

	bool is_at_detent() const;

	std::vector<uint16_t> m_detents;
	uint16_t m_max_pos;
	attotime m_step_period;

	output_finder<> m_reel_out;
	devcb_write_line m_state_cb;

	reel_state m_state;
	uint16_t m_pos;
	dir m_direction;
	emu_timer *m_move_timer;
};

DECLARE_DEVICE_TYPE(EM_REEL, em_reel_device)

#endif // MAME_MACHINE_EM_REEL_H
