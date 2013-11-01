// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Intel 80130 iRMX Operating System Processor emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __I80130__
#define __I80130__

#include "emu.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"



///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

#define MCFG_I80130_ADD(_tag, _clock, _irq) \
	MCFG_DEVICE_ADD(_tag, I80130, _clock) \
	downcast<i80130_device *>(device)->set_irq_callback(DEVCB2_##_irq);

#define MCFG_I80130_ACK_CALLBACK(_ack) \
	downcast<i80130_device *>(device)->set_ack_callback(DEVCB2_##_ack);

#define MCFG_I80130_LIR_CALLBACK(_lir) \
	downcast<i80130_device *>(device)->set_lir_callback(DEVCB2_##_lir);

#define MCFG_I80130_SYSTICK_CALLBACK(_systick) \
	downcast<i80130_device *>(device)->set_systick_callback(DEVCB2_##_systick);

#define MCFG_I80130_DELAY_CALLBACK(_delay) \
	downcast<i80130_device *>(device)->set_delay_callback(DEVCB2_##_delay);

#define MCFG_I80130_BAUD_CALLBACK(_baud) \
	downcast<i80130_device *>(device)->set_baud_callback(DEVCB2_##_baud);



///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> i80130_device

class i80130_device :  public device_t
{
public:
	// construction/destruction
	i80130_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _irq> void set_irq_callback(_irq irq) { m_write_irq.set_callback(irq); }
	template<class _ack> void set_ack_callback(_ack ack) { m_write_ack.set_callback(ack); }
	template<class _lir> void set_lir_callback(_lir lir) { m_write_lir.set_callback(lir); }
	template<class _systick> void set_systick_callback(_systick systick) { m_write_systick.set_callback(systick); }
	template<class _delay> void set_delay_callback(_delay delay) { m_write_delay.set_callback(delay); }
	template<class _baud> void set_baud_callback(_baud baud) { m_write_baud.set_callback(baud); }

	virtual DECLARE_ADDRESS_MAP(rom_map, 16);
	virtual DECLARE_ADDRESS_MAP(io_map, 16);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

	UINT8 inta_r() { return m_pic->inta_r(); }

	DECLARE_WRITE_LINE_MEMBER( ir0_w ) { m_pic->ir0_w(state); }
	DECLARE_WRITE_LINE_MEMBER( ir1_w ) { m_pic->ir1_w(state); }
	DECLARE_WRITE_LINE_MEMBER( ir2_w ) { m_pic->ir2_w(state); }
	DECLARE_WRITE_LINE_MEMBER( ir3_w ) { m_pic->ir3_w(state); }
	DECLARE_WRITE_LINE_MEMBER( ir4_w ) { m_pic->ir4_w(state); }
	DECLARE_WRITE_LINE_MEMBER( ir5_w ) { m_pic->ir5_w(state); }
	DECLARE_WRITE_LINE_MEMBER( ir6_w ) { m_pic->ir6_w(state); }
	DECLARE_WRITE_LINE_MEMBER( ir7_w ) { m_pic->ir7_w(state); }

	DECLARE_WRITE_LINE_MEMBER( irq_w ) { m_write_irq(state); }
	DECLARE_WRITE_LINE_MEMBER( systick_w ) { m_write_systick(state); }
	DECLARE_WRITE_LINE_MEMBER( delay_w ) { m_write_delay(state); }
	DECLARE_WRITE_LINE_MEMBER( baud_w ) { m_write_baud(state); }

	DECLARE_READ16_MEMBER( io_r );
	DECLARE_WRITE16_MEMBER( io_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	required_device<pic8259_device> m_pic;
	required_device<pit8254_device> m_pit;

	devcb2_write_line m_write_irq;
	devcb2_write_line m_write_ack;
	devcb2_write_line m_write_lir;
	devcb2_write_line m_write_systick;
	devcb2_write_line m_write_delay;
	devcb2_write_line m_write_baud;
};


// device type definition
extern const device_type I80130;



#endif
