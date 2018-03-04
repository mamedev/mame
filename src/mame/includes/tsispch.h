// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
/***************************************************************************

        tsispch.h

****************************************************************************/

#ifndef MAME_INCLUDES_TSISPCH_H
#define MAME_INCLUDES_TSISPCH_H

#pragma once

#include "machine/pic8259.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class tsispch_state : public driver_device
{
public:
	tsispch_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dsp(*this, "dsp"),
		m_terminal(*this, TERMINAL_TAG),
		m_pic(*this, "pic8259")
	{
	}

	DECLARE_READ8_MEMBER(dsw_r);
	DECLARE_WRITE8_MEMBER(peripheral_w);
	DECLARE_READ16_MEMBER(dsp_data_r);
	DECLARE_WRITE16_MEMBER(dsp_data_w);
	DECLARE_READ16_MEMBER(dsp_status_r);
	DECLARE_WRITE16_MEMBER(dsp_status_w);
	DECLARE_DRIVER_INIT(prose2k);
	DECLARE_WRITE_LINE_MEMBER(i8251_rxrdy_int);
	DECLARE_WRITE_LINE_MEMBER(i8251_txempty_int);
	DECLARE_WRITE_LINE_MEMBER(i8251_txrdy_int);
	DECLARE_WRITE_LINE_MEMBER(dsp_to_8086_p0_w);
	DECLARE_WRITE_LINE_MEMBER(dsp_to_8086_p1_w);

	void prose2k(machine_config &config);
	void dsp_data_map(address_map &map);
	void dsp_prg_map(address_map &map);
	void i8086_io(address_map &map);
	void i8086_mem(address_map &map);
protected:
	virtual void machine_reset() override;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_dsp;
	required_device<generic_terminal_device> m_terminal;
	required_device<pic8259_device> m_pic;

	uint8_t m_paramReg;           // status leds and resets and etc
};

#endif // MAME_INCLUDES_TSISPCH_H
