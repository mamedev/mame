// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
/***************************************************************************

        pes.h

****************************************************************************/

#pragma once

#ifndef MAME_INCLUDES_PES_H
#define MAME_INCLUDES_PES_H

#include "machine/terminal.h"
#include "sound/tms5220.h"
#include "cpu/mcs51/mcs51.h"


class pes_state : public driver_device
{
public:
	pes_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, "terminal"),
		m_speech(*this, "tms5220")
	{
	}

	required_device<i80c31_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	required_device<tms5220_device> m_speech;

	uint8_t m_wsstate;            // /WS
	uint8_t m_rsstate;            // /RS
	uint8_t m_port3_state;        // Port3 state as last written
	uint8_t m_infifo[32];         // input fifo
	uint8_t m_infifo_tail_ptr;        // " tail
	uint8_t m_infifo_head_ptr;        // " head

	virtual void machine_reset() override;
	DECLARE_WRITE8_MEMBER(rsq_wsq_w);
	DECLARE_WRITE8_MEMBER(port1_w);
	DECLARE_WRITE8_MEMBER(port3_w);
	DECLARE_READ8_MEMBER(port1_r);
	DECLARE_READ8_MEMBER(port3_r);
	void pes_kbd_input(u8 data);
	DECLARE_READ8_MEMBER(data_to_i8031);
	DECLARE_WRITE8_MEMBER(data_from_i8031);
	void pes(machine_config &config);
	void i80c31_io(address_map &map);
	void i80c31_mem(address_map &map);
};


#endif // MAME_INCLUDES_PES_H
