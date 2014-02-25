// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS Technology 6551 Asynchronous Communication Interface Adapter

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                   GND   1 |*    \_/     | 28  R/_W
                   CS0   2 |             | 27  phi2
                  _CS1   3 |             | 26  _IRQ
                  _RES   4 |             | 25  DB7
                   RxC   5 |             | 24  DB6
                 XTAL1   6 |             | 23  DB5
                 XTAL2   7 |   MOS6551   | 22  DB4
                  _RTS   8 |             | 21  DB3
                  _CTS   9 |             | 20  DB2
                   TxD  10 |             | 19  DB1
                  _DTR  11 |             | 18  DB0
                   RxD  12 |             | 17  _DBR
                   RS0  13 |             | 16  _DCD
                   RS1  14 |_____________| 15  Vcc

**********************************************************************/

#pragma once

#ifndef __MOS6551__
#define __MOS6551__

#include "emu.h"

// MOS 6551s reset with the RIE bit set in the command register
#define MOS6551_TYPE_MOS		(0)
// Rockwell (and Synertek) reset with all bits clear in the command register
#define MOS6551_TYPE_ROCKWELL	(1)

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MOS6551_IRQ_HANDLER(_devcb) \
	devcb = &mos6551_device::set_irq_handler(*device, DEVCB2_##_devcb);

#define MCFG_MOS6551_TXD_HANDLER(_devcb) \
	devcb = &mos6551_device::set_txd_handler(*device, DEVCB2_##_devcb);

#define MCFG_MOS6551_RTS_HANDLER(_devcb) \
	devcb = &mos6551_device::set_rts_handler(*device, DEVCB2_##_devcb);

#define MCFG_MOS6551_DTR_HANDLER(_devcb) \
	devcb = &mos6551_device::set_dtr_handler(*device, DEVCB2_##_devcb);

#define MCFG_MOS6551_TYPE(_type) \
	mos6551_device::static_set_type(*device, _type);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mos6551_device

class mos6551_device :  public device_t,
						public device_serial_interface
{
public:
	// construction/destruction
	mos6551_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb2_base &set_irq_handler(device_t &device, _Object object) { return downcast<mos6551_device &>(device).m_irq_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_txd_handler(device_t &device, _Object object) { return downcast<mos6551_device &>(device).m_txd_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_rts_handler(device_t &device, _Object object) { return downcast<mos6551_device &>(device).m_rts_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_dtr_handler(device_t &device, _Object object) { return downcast<mos6551_device &>(device).m_dtr_handler.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( rxd_w );
	DECLARE_WRITE_LINE_MEMBER( rxc_w );
	DECLARE_WRITE_LINE_MEMBER( cts_w );
	DECLARE_WRITE_LINE_MEMBER( dsr_w );
	DECLARE_WRITE_LINE_MEMBER( dcd_w );

	void set_rxc(int clock);

	// inline configuration helpers
	static void static_set_type(device_t &device, int type);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_serial_interface overrides
	virtual void tra_callback();
	virtual void tra_complete();
	virtual void rcv_complete();

	enum
	{
		CTRL_BRG_16X_EXTCLK = 0,
		CTRL_BRG_50,
		CTRL_BRG_75,
		CTRL_BRG_109_92,
		CTRL_BRG_134_58,
		CTRL_BRG_150,
		CTRL_BRG_300,
		CTRL_BRG_600,
		CTRL_BRG_1200,
		CTRL_BRG_1800,
		CTRL_BRG_2400,
		CTRL_BRG_3600,
		CTRL_BRG_4800,
		CTRL_BRG_7200,
		CTRL_BRG_9600,
		CTRL_BRG_19200,
		CTRL_BRG_MASK = 0x0f,

		CTRL_RXC_EXT = 0x00,
		CTRL_RXC_BRG = 0x10,
		CTRL_RXC_MASK = 0x10,

		CTRL_WL_8 = 0x00,
		CTRL_WL_7 = 0x20,
		CTRL_WL_6 = 0x40,
		CTRL_WL_5 = 0x60,
		CTRL_WL_MASK = 0x60,

		CTRL_SB_1 = 0x00,
		CTRL_SB_2 = 0x80,
		CTRL_SB_MASK = 0x80
	};

	enum
	{
		CMD_DTR = 0x01,

		CMD_RIE = 0x02,

		CMD_TC_RTS_HI = 0x00,
		CMD_TC_TIE_RTS_LO = 0x04,
		CMD_TC_RTS_LO = 0x08,
		CMD_TC_BRK = 0x0c,
		CMD_TC_MASK = 0x0c,

		CMD_ECHO = 0x10,

		CMD_PARITY = 0x20,
		CMD_PARITY_ODD = 0x00,
		CMD_PARITY_EVEN = 0x40,
		CMD_PARITY_MARK = 0x80,
		CMD_PARITY_SPACE = 0xc0,
		CMD_PARITY_MASK = 0xc0
	};

	enum
	{
		ST_PE = 0x01,
		ST_FE = 0x02,
		ST_OR = 0x04,
		ST_RDRF = 0x08,
		ST_TDRE = 0x10,
		ST_DCD = 0x20,
		ST_DSR = 0x40,
		ST_IRQ = 0x80
	};

	void update_serial();

	devcb2_write_line m_irq_handler;
	devcb2_write_line m_txd_handler;
	devcb2_write_line m_rts_handler;
	devcb2_write_line m_dtr_handler;

	UINT8 m_ctrl;
	UINT8 m_cmd;
	UINT8 m_st;
	UINT8 m_tdr;

	int m_ext_rxc;
	int m_cts;
	int m_dsr;
	int m_dcd;

	int m_chip_type;

	static const int brg_divider[16];
};


// device type definition
extern const device_type MOS6551;



#endif
