// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
/***************************************************************************

        pes.h

****************************************************************************/

#pragma once

#ifndef _PES_H_
#define _PES_H_

#include "machine/terminal.h"
#include "sound/tms5220.h"
#include "cpu/mcs51/mcs51.h"

#define TERMINAL_TAG "terminal"

class pes_state : public driver_device
{
public:
	pes_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG),
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
	void rsws_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void port1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void port3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t port1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t port3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pes_kbd_input(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t data_to_i8031(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void data_from_i8031(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
};


#endif  // _PES_H_
