/***************************************************************************

        tsispch.h

****************************************************************************/

#pragma once

#ifndef _TSISPCH_H_
#define _TSISPCH_H_

#include "machine/terminal.h"

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
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_dsp;
	required_device<device_t> m_terminal;
	required_device<device_t> m_uart;
	required_device<device_t> m_pic;

	UINT8 m_infifo[32];			// input fifo
	UINT8 m_infifo_tail_ptr;		// " tail
	UINT8 m_infifo_head_ptr;		// " head
	UINT8 m_paramReg;			// status leds and resets and etc

	virtual void machine_reset();
	DECLARE_WRITE8_MEMBER(i8251_rxd);
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
	DECLARE_WRITE_LINE_MEMBER(pic8259_set_int_line);
};


#endif	// _TSISPCH_H_
