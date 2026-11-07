// license:BSD-3-Clause
// copyright-holders:gregre365
#include "emu.h"

#include "wildcats_reel.h"

DEFINE_DEVICE_TYPE(WILDCATS_REEL, wildcats_reel_device, "wildcats_reel", "Wild Cats Reel")

wildcats_reel_device::wildcats_reel_device(
		const machine_config &mconfig,
		const char *tag,
		device_t *owner,
		int16_t start_index,
		int16_t end_index,
		int16_t index_pattern,
		uint8_t init_phase,
		int16_t max_steps)
	: wildcats_reel_device(mconfig, tag, owner, uint32_t(0))
{
	set_start_index(start_index);
	set_end_index(end_index);
	set_index_pattern(index_pattern);
	set_init_phase(init_phase);
	set_max_steps(max_steps);
}

wildcats_reel_device::wildcats_reel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: stepper_device(mconfig, WILDCATS_REEL, tag, owner, clock)
{
}

void wildcats_reel_device::advance_phase()
{
	switch (m_pattern)
	{
		case 0x83: //  0     0     1     0
			m_phase = 7;
			break;
		case 0xc1: //  0     1     1     0
			m_phase = 6;
			break;
		case 0xe0: //  0     1     0     0
			m_phase = 5;
			break;
		case 0x70: //  0     1     0     1
			m_phase = 4;
			break;
		case 0x38: //  0     0     0     1
			m_phase = 3;
			break;
		case 0x1c: //  1     0     0     1
			m_phase = 2;
			break;
		case 0x0e: //  1     0     0     0
			m_phase = 1;
			break;
		case 0x07: //  1     0     1     0
			m_phase = 0;
			break;
		default:
			stepper_device::advance_phase();
			break;
	}
}
