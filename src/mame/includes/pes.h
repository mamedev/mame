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
	pes_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG),
		m_speech(*this, "tms5220")
	{
	}

	required_device<i80c31_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	required_device<tms5220_device> m_speech;

	UINT8 m_wsstate;            // /WS
	UINT8 m_rsstate;            // /RS
	UINT8 m_port3_state;        // Port3 state as last written
	UINT8 m_infifo[32];         // input fifo
	UINT8 m_infifo_tail_ptr;        // " tail
	UINT8 m_infifo_head_ptr;        // " head

	virtual void machine_reset() override;
	DECLARE_WRITE8_MEMBER(rsws_w);
	DECLARE_WRITE8_MEMBER(port1_w);
	DECLARE_WRITE8_MEMBER(port3_w);
	DECLARE_READ8_MEMBER(port1_r);
	DECLARE_READ8_MEMBER(port3_r);
	DECLARE_DRIVER_INIT(pes);
	DECLARE_WRITE8_MEMBER(pes_kbd_input);
	DECLARE_READ8_MEMBER(data_to_i8031);
	DECLARE_WRITE8_MEMBER(data_from_i8031);
};


#endif  // _PES_H_
