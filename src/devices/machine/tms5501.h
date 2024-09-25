// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    TMS5501 Multifunction Input/Output Controller emulation

**********************************************************************
                           _____   _____
                   Vbb   1 |*    \_/     | 40  XMT
                   Vcc   2 |             | 39  XI0
                   Vdd   3 |             | 38  XI1
                   VSS   4 |             | 37  XI2
                   RCV   5 |             | 36  XI3
                    D7   6 |             | 35  XI4
                    D6   7 |             | 34  XI5
                    D5   8 |             | 33  XI6
                    D4   9 |             | 32  XI7
                    D3  10 |             | 31  _XO7
                    D2  11 |   TMS5501   | 30  _XO6
                    D1  12 |             | 29  _XO5
                    D0  13 |             | 28  _XO4
                    A0  14 |             | 27  _XO3
                    A1  15 |             | 26  _XO2
                    A2  16 |             | 25  _XO1
                    A3  17 |             | 24  _XO0
                    CE  18 |             | 23  INT
                  SYNC  19 |             | 22  SENS
                  phi1  20 |_____________| 21  phi2

**********************************************************************/

#ifndef MAME_MACHINE_TMS5501_H
#define MAME_MACHINE_TMS5501_H

#pragma once

#include "diserial.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tms5501_device

class tms5501_device :  public device_t,
						public device_serial_interface
{
public:
	// construction/destruction
	tms5501_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto int_callback() { return m_write_int.bind(); }
	auto xmt_callback() { return m_write_xmt.bind(); }
	auto xi_callback() { return m_read_xi.bind(); }
	auto xo_callback() { return m_write_xo.bind(); }

	void rcv_w(int state);

	void sens_w(int state);
	void xi7_w(int state);

	uint8_t get_vector();

	virtual void io_map(address_map &map) ATTR_COLD;

	uint8_t rb_r();
	uint8_t xi_r();
	uint8_t rst_r();
	uint8_t sta_r();
	void cmd_w(uint8_t data);
	void rr_w(uint8_t data);
	void tb_w(uint8_t data);
	void xo_w(uint8_t data);
	void mr_w(uint8_t data);
	void tmr_w(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_serial_interface overrides
	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_complete() override;

	TIMER_CALLBACK_MEMBER(timer_expired);

private:
	enum
	{
		TIMER_1 = 0,
		TIMER_2,
		TIMER_3,
		TIMER_4,
		TIMER_5
	};

	enum
	{
		IRQ_TMR1 = 0x01,
		IRQ_TMR2 = 0x02,
		IRQ_SENS = 0x04,
		IRQ_TMR3 = 0x08,
		IRQ_RB   = 0x10,
		IRQ_TB   = 0x20,
		IRQ_TMR4 = 0x40,
		IRQ_TMR5 = 0x80,
		IRQ_XI7  = 0x80
	};

	enum
	{
		STA_FE   = 0x01,
		STA_OE   = 0x02,
		STA_SR   = 0x04,
		STA_RBL  = 0x08,
		STA_XBE  = 0x10,
		STA_IP   = 0x20,
		STA_FBD  = 0x40,
		STA_SBD  = 0x80
	};

	enum
	{
		CMD_RST  = 0x01,
		CMD_BRK  = 0x02,
		CMD_XI7  = 0x04,
		CMD_IAE  = 0x08,
		CMD_TST1 = 0x10,
		CMD_TST2 = 0x20
	};

	enum
	{
		RR_110   = 0x01,
		RR_150   = 0x02,
		RR_300   = 0x04,
		RR_1200  = 0x08,
		RR_2400  = 0x10,
		RR_4800  = 0x20,
		RR_9600  = 0x40,
		RR_STOP  = 0x80
	};

	static const uint8_t rst_vector[];

	void set_interrupt(uint8_t mask);
	void check_interrupt();

	devcb_write_line m_write_int;
	devcb_write_line m_write_xmt;
	devcb_read8 m_read_xi;
	devcb_write8 m_write_xo;

	uint8_t m_irq;
	uint8_t m_rb;
	uint8_t m_sta;
	uint8_t m_cmd;
	uint8_t m_rr;
	uint8_t m_tb;
	uint8_t m_mr;

	int m_sens;
	int m_xi7;

	emu_timer *m_timer[5];
};


// device type definition
DECLARE_DEVICE_TYPE(TMS5501, tms5501_device)

#endif // MAME_MACHINE_TMS5501_H
