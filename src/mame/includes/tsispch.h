// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
/***************************************************************************

        tsispch.h

****************************************************************************/

#pragma once

#ifndef _TSISPCH_H_
#define _TSISPCH_H_

#include "machine/i8251.h"
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
		m_uart(*this, "i8251a_u15"),
		m_pic(*this, "pic8259")
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_dsp;
	required_device<generic_terminal_device> m_terminal;
	required_device<i8251_device> m_uart;
	required_device<pic8259_device> m_pic;

	uint8_t m_paramReg;           // status leds and resets and etc

	virtual void machine_reset() override;
	void i8251_rxd(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t dsw_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void peripheral_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t dsp_data_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dsp_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dsp_status_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dsp_status_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void init_prose2k();
	void i8251_rxrdy_int(int state);
	void i8251_txempty_int(int state);
	void i8251_txrdy_int(int state);
	void dsp_to_8086_p0_w(int state);
	void dsp_to_8086_p1_w(int state);
};


#endif  // _TSISPCH_H_
