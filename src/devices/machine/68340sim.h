// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_MACHINE_68340SIM_H
#define MAME_MACHINE_68340SIM_H

#pragma once

class m68340_sim
{
public:
	// Chip selects
	uint32_t m_am[4]; // chip select address mask + control, unaffected by reset
	uint32_t m_ba[4]; // chip select base address + control, unaffected by reset

	// Ports             Reset values
	uint8_t m_porta;  // unaffected by reset
	uint8_t m_ddra;   // 0x00
	uint8_t m_ppara1; // 0xff
	uint8_t m_ppara2; // 0x00
	uint8_t m_portb;  // unaffected by reset
	uint8_t m_ddrb;   // 0x00
	uint8_t m_pparb;  // 0xff

	void reset();
};

#endif // MAME_MACHINE_68340SIM_H
