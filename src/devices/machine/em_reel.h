// license:BSD-3-Clause
// copyright-holders:SomeRandomGuyIdk
/**********************************************************************

	Electromechanical reels for slot machines

**********************************************************************/

#ifndef MAME_MACHINE_EM_REEL_H
#define MAME_MACHINE_EM_REEL_H

#pragma once

class em_reel_device : public device_t
{
public:
	em_reel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	em_reel_device(const machine_config &mconfig, const char *tag, device_t *owner, 
				   uint16_t steps, uint16_t numstops, double speed, uint8_t direction = REVERSE) :
				   em_reel_device(mconfig, tag, owner, 0)
	{
		set_max_pos(steps);
		set_steps_per_detent(numstops);
		set_speed(speed);
		set_direction(direction);
	}

	// Start and stop the reel
	void set_state(uint8_t state);
	// Set the direction the reel moves in
	void set_direction(uint8_t direction) { m_direction = direction; }
	// Set the speed of the reel (revolutions/second)
	void set_speed(double speed) { m_step_period = attotime::from_double(1.0 / speed / m_max_pos); }
	// Get the reel's current step position
	uint16_t get_pos() { return m_pos; }

	// Movement state callback, used for playing samples
	auto state_changed_callback() { return m_state_cb.bind(); }

	void set_max_pos(uint16_t steps) { m_max_pos = steps; }
	void set_steps_per_detent(uint16_t numstops);

	enum
	{
		FORWARD = 0, // Steps count up
		REVERSE // Steps count down
	};

protected:
	virtual void device_start() override;

private:
	uint8_t m_state;
	uint16_t m_pos;
	emu_timer *m_move_timer;
	uint16_t m_max_pos;
	uint16_t m_steps_per_detent;
	attotime m_step_period;
	uint8_t m_direction;

	TIMER_CALLBACK_MEMBER(move);

	enum
	{
		REEL_STOPPED = 0,
		REEL_SPINNING,
		REEL_STOPPING
	};

	output_finder<> m_reel_out;
	devcb_write8 m_state_cb;
};

DECLARE_DEVICE_TYPE(EM_REEL, em_reel_device)

#endif // MAME_MACHINE_EM_REEL_H
