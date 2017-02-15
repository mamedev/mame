// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy 2000 keyboard emulation

*********************************************************************/

#pragma once

#ifndef __TANDY2K_KEYBOARD__
#define __TANDY2K_KEYBOARD__

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
	tandy2k_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _Object> static devcb_base &set_clock_wr_callback(device_t &device, _Object object) { return downcast<tandy2k_keyboard_device &>(device).m_write_clock.set_callback(object); }
	template<class _Object> static devcb_base &set_data_wr_callback(device_t &device, _Object object) { return downcast<tandy2k_keyboard_device &>(device).m_write_data.set_callback(object); }

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

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
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	enum
	{
		LED_1 = 0,
		LED_2
	};

	required_device<cpu_device> m_maincpu;
	required_ioport_array<12> m_y;

	devcb_write_line   m_write_clock;
	devcb_write_line   m_write_data;

	uint16_t m_keylatch;

	int m_clock;
	int m_data;
};


// device type definition
extern const device_type TANDY2K_KEYBOARD;



#endif
