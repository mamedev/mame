// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy 2000 keyboard emulation

*********************************************************************/

#pragma once

#ifndef __TANDY2K_KEYBOARD__
#define __TANDY2K_KEYBOARD__

#include "emu.h"
#include "cpu/mcs48/mcs48.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define TANDY2K_KEYBOARD_TAG    "tandy2kb"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_TANDY2000_KEYBOARD_CLOCK_CALLBACK(_write) \
	devcb = &tandy2k_keyboard_device::set_clock_wr_callback(*device, DEVCB_##_write);

#define MCFG_TANDY2000_KEYBOARD_DATA_CALLBACK(_write) \
	devcb = &tandy2k_keyboard_device::set_data_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tandy2k_keyboard_device

class tandy2k_keyboard_device :  public device_t
{
public:
	// construction/destruction
	tandy2k_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_clock_wr_callback(device_t &device, _Object object) { return downcast<tandy2k_keyboard_device &>(device).m_write_clock.set_callback(object); }
	template<class _Object> static devcb_base &set_data_wr_callback(device_t &device, _Object object) { return downcast<tandy2k_keyboard_device &>(device).m_write_data.set_callback(object); }

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	DECLARE_WRITE_LINE_MEMBER( power_w );
	DECLARE_WRITE_LINE_MEMBER( reset_w );
	DECLARE_WRITE_LINE_MEMBER( busy_w );
	DECLARE_READ_LINE_MEMBER( data_r );

	// not really public
	DECLARE_READ8_MEMBER( kb_p1_r );
	DECLARE_WRITE8_MEMBER( kb_p1_w );
	DECLARE_WRITE8_MEMBER( kb_p2_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:
	enum
	{
		LED_1 = 0,
		LED_2
	};

	required_device<cpu_device> m_maincpu;
	required_ioport m_y0;
	required_ioport m_y1;
	required_ioport m_y2;
	required_ioport m_y3;
	required_ioport m_y4;
	required_ioport m_y5;
	required_ioport m_y6;
	required_ioport m_y7;
	required_ioport m_y8;
	required_ioport m_y9;
	required_ioport m_y10;
	required_ioport m_y11;

	devcb_write_line   m_write_clock;
	devcb_write_line   m_write_data;

	UINT16 m_keylatch;

	int m_clock;
	int m_data;
};


// device type definition
extern const device_type TANDY2K_KEYBOARD;



#endif
