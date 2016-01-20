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

#pragma once

#ifndef __TMS5501__
#define __TMS5501__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_TMS5501_IRQ_CALLBACK(_write) \
	devcb = &tms5501_device::set_irq_wr_callback(*device, DEVCB_##_write);

#define MCFG_TMS5501_XMT_CALLBACK(_write) \
	devcb = &tms5501_device::set_xmt_wr_callback(*device, DEVCB_##_write);

#define MCFG_TMS5501_XI_CALLBACK(_read) \
	devcb = &tms5501_device::set_xi_rd_callback(*device, DEVCB_##_read);

#define MCFG_TMS5501_XO_CALLBACK(_write) \
	devcb = &tms5501_device::set_xo_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tms5501_device

class tms5501_device :  public device_t,
						public device_serial_interface
{
public:
	// construction/destruction
	tms5501_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_irq_wr_callback(device_t &device, _Object object) { return downcast<tms5501_device &>(device).m_write_irq.set_callback(object); }
	template<class _Object> static devcb_base &set_xmt_wr_callback(device_t &device, _Object object) { return downcast<tms5501_device &>(device).m_write_xmt.set_callback(object); }
	template<class _Object> static devcb_base &set_xi_rd_callback(device_t &device, _Object object) { return downcast<tms5501_device &>(device).m_read_xi.set_callback(object); }
	template<class _Object> static devcb_base &set_xo_wr_callback(device_t &device, _Object object) { return downcast<tms5501_device &>(device).m_write_xo.set_callback(object); }

	virtual DECLARE_ADDRESS_MAP(io_map, 8);

	DECLARE_WRITE_LINE_MEMBER( rcv_w );

	DECLARE_WRITE_LINE_MEMBER( sens_w );
	DECLARE_WRITE_LINE_MEMBER( xi7_w );

	UINT8 get_vector();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_serial_interface overrides
	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_complete() override;

	DECLARE_READ8_MEMBER( rb_r );
	DECLARE_READ8_MEMBER( xi_r );
	DECLARE_READ8_MEMBER( rst_r );
	DECLARE_READ8_MEMBER( sta_r );
	DECLARE_WRITE8_MEMBER( cmd_w );
	DECLARE_WRITE8_MEMBER( rr_w );
	DECLARE_WRITE8_MEMBER( tb_w );
	DECLARE_WRITE8_MEMBER( xo_w );
	DECLARE_WRITE8_MEMBER( mr_w );
	DECLARE_WRITE8_MEMBER( tmr_w );

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

	static const UINT8 rst_vector[];

	void set_interrupt(UINT8 mask);
	void check_interrupt();

	devcb_write_line m_write_irq;
	devcb_write_line m_write_xmt;
	devcb_read8 m_read_xi;
	devcb_write8 m_write_xo;

	UINT8 m_irq;
	UINT8 m_rb;
	UINT8 m_sta;
	UINT8 m_cmd;
	UINT8 m_rr;
	UINT8 m_tb;
	UINT8 m_mr;

	int m_sens;
	int m_xi7;

	emu_timer *m_timer[5];
};


// device type definition
extern const device_type TMS5501;



#endif
