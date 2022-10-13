// license:BSD-3-Clause
// copyright-holders:SomeRandomGuyIdk
/**********************************************************************

	Electromechanical reels for slot machines

**********************************************************************/

#ifndef MAME_MACHINE_EM_REEL_H
#define MAME_MACHINE_EM_REEL_H

#pragma once

#include "sound/samples.h"

class em_reel_device : public device_t
{
public:
	em_reel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	em_reel_device(const machine_config &mconfig, const char *tag, device_t *owner, 
				   uint8_t numsymbols, attotime speed_period, uint8_t direction = REVERSE) :
				   em_reel_device(mconfig, tag, owner, 0)
	{
		set_max_pos(numsymbols);
		set_speed(speed_period);
		set_direction(direction);
	}

	// Start and stop the reel
	void set_state(uint8_t state);
	// Set the direction the reel moves in
	void set_direction(uint8_t direction) { m_direction = direction; }
	// Set the speed of the reel, or how much time it takes to move one 'step'
	void set_speed(attotime period) { m_speed_period = period; }
	// Get which symbol the reel is on (0 if it's inbetween)
	uint8_t read_pos();
	// Get the state of the symbol opto sensor on Bellfruit reels
	bool read_bfm_symbol_opto();
	// Get the state of the reel opto sensor on Bellfruit reels
	bool read_bfm_reel_opto();

	void set_max_pos(uint8_t numsymbols) { m_max_pos = numsymbols * STEPS_PER_SYMBOL; }

	enum
	{
		FORWARD = 0, // Symbol position counts up
		REVERSE // Symbol position counts down
	};

protected:
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	uint8_t m_state;
	uint16_t m_pos;
	emu_timer *m_move_timer;
	uint8_t m_direction;
	attotime m_speed_period;
	uint16_t m_max_pos;

	TIMER_CALLBACK_MEMBER(move);

	enum
	{
		STEPS_PER_SYMBOL = 20, // Completely arbitrary number
		REEL_STOPPED = 0,
		REEL_SPINNING,
		REEL_STOPPING,
		SAMPLE_START = 0,
		SAMPLE_STOP
	};

	output_finder<> m_reel_out;
	required_device<samples_device> m_samples;
};

DECLARE_DEVICE_TYPE(EM_REEL, em_reel_device)

#endif // MAME_MACHINE_EM_REEL_H
