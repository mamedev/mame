// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Amiga Keyboard

    We currently emulate the Amiga 500 keyboard controller, which was
    also used in later Amiga 2000 keyboards.

***************************************************************************/

#pragma once

#ifndef __AMIGAKBD_H__
#define __AMIGAKBD_H__

#include "emu.h"
#include "cpu/m6502/m6502.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_AMIGA_KEYBOARD_KCLK_CALLBACK(_write) \
	devcb = &amigakbd_device::set_kclk_wr_callback(*device, DEVCB_##_write);

#define MCFG_AMIGA_KEYBOARD_KDAT_CALLBACK(_write) \
	devcb = &amigakbd_device::set_kdat_wr_callback(*device, DEVCB_##_write);

#define MCFG_AMIGA_KEYBOARD_KRST_CALLBACK(_write) \
	devcb = &amigakbd_device::set_krst_wr_callback(*device, DEVCB_##_write);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> amigakbd_device

class amigakbd_device : public device_t
{
public:
	// construction/destruction
	amigakbd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_kclk_wr_callback(device_t &device, _Object object)
		{ return downcast<amigakbd_device &>(device).m_write_kclk.set_callback(object); }
	template<class _Object> static devcb_base &set_kdat_wr_callback(device_t &device, _Object object)
		{ return downcast<amigakbd_device &>(device).m_write_kdat.set_callback(object); }
	template<class _Object> static devcb_base &set_krst_wr_callback(device_t &device, _Object object)
		{ return downcast<amigakbd_device &>(device).m_write_krst.set_callback(object); }

	DECLARE_WRITE_LINE_MEMBER( kdat_w );

	// 6500/1 internal
	DECLARE_READ8_MEMBER( port_a_r );
	DECLARE_WRITE8_MEMBER( port_a_w );
	DECLARE_READ8_MEMBER( port_b_r );
	DECLARE_WRITE8_MEMBER( port_b_w );
	DECLARE_WRITE8_MEMBER( port_c_w );
	DECLARE_WRITE8_MEMBER( port_d_w );
	DECLARE_WRITE8_MEMBER( latch_w );
	DECLARE_READ8_MEMBER( counter_r );
	DECLARE_WRITE8_MEMBER( transfer_latch_w );
	DECLARE_WRITE8_MEMBER( clear_pa0_detect );
	DECLARE_WRITE8_MEMBER( clear_pa1_detect );
	DECLARE_READ8_MEMBER( control_r );
	DECLARE_WRITE8_MEMBER( control_w );

	DECLARE_INPUT_CHANGED_MEMBER( check_reset );

protected:
	// device-level overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	enum
	{
		// counter modes
		COUNTER_INTERVAL = 0x00,
		COUNTER_PULSE = 0x01,
		COUNTER_EVENT = 0x02,
		COUNTER_PWM = 0x03,

		// interrupt enables
		PA1_INT_ENABLED     = 0x04,
		PA0_INT_ENABLED     = 0x08,
		COUNTER_INT_ENABLED = 0x10,

		// status
		PA1_NEGATIVE_EDGE = 0x20,
		PA0_POSITIVE_EDGE = 0x40,
		COUNTER_OVERFLOW  = 0x80
	};

	void update_irqs();

	devcb_write_line m_write_kclk;
	devcb_write_line m_write_kdat;
	devcb_write_line m_write_krst;

	required_device<m6502_device> m_mpu;

	required_ioport m_special;
	required_ioport m_row_d6;
	required_ioport m_row_d5;
	required_ioport m_row_d4;
	required_ioport m_row_d3;
	required_ioport m_row_d2;
	required_ioport m_row_d1;
	required_ioport m_row_d0;
	required_ioport m_row_c7;
	required_ioport m_row_c6;
	required_ioport m_row_c5;
	required_ioport m_row_c4;
	required_ioport m_row_c3;
	required_ioport m_row_c2;
	required_ioport m_row_c1;
	required_ioport m_row_c0;

	emu_timer *m_timer;
	emu_timer *m_watchdog;
	emu_timer *m_reset;

	int m_kdat;
	int m_kclk;

	UINT8 m_port_c;
	UINT8 m_port_d;
	UINT16 m_latch;
	UINT16 m_counter;
	UINT8 m_control;
};

// device type definition
extern const device_type AMIGAKBD;

#endif // __AMIGAKBD_H__
