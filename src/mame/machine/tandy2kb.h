// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy 2000 keyboard emulation

*********************************************************************/

#ifndef MAME_MACHINE_TANDY2KB_H
#define MAME_MACHINE_TANDY2KB_H

#pragma once

#include "cpu/mcs48/mcs48.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define TANDY2K_KEYBOARD_TAG    "tandy2kb"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_TANDY2000_KEYBOARD_CLOCK_CALLBACK(_write) \
	devcb = &downcast<tandy2k_keyboard_device &>(*device).set_clock_wr_callback(DEVCB_##_write);

#define MCFG_TANDY2000_KEYBOARD_DATA_CALLBACK(_write) \
	devcb = &downcast<tandy2k_keyboard_device &>(*device).set_data_wr_callback(DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tandy2k_keyboard_device

class tandy2k_keyboard_device :  public device_t
{
public:
	// construction/destruction
	tandy2k_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> devcb_base &set_clock_wr_callback(Object &&cb) { return m_write_clock.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_data_wr_callback(Object &&cb) { return m_write_data.set_callback(std::forward<Object>(cb)); }

	DECLARE_WRITE_LINE_MEMBER( power_w );
	DECLARE_WRITE_LINE_MEMBER( reset_w );
	DECLARE_WRITE_LINE_MEMBER( busy_w );
	DECLARE_READ_LINE_MEMBER( data_r );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

private:
	enum
	{
		LED_1 = 0,
		LED_2
	};

	required_device<cpu_device> m_maincpu;
	required_ioport_array<12> m_y;
	output_finder<2> m_led;

	devcb_write_line   m_write_clock;
	devcb_write_line   m_write_data;

	uint16_t m_keylatch;

	int m_clock;
	int m_data;

	DECLARE_READ8_MEMBER( kb_p1_r );
	DECLARE_WRITE8_MEMBER( kb_p1_w );
	DECLARE_WRITE8_MEMBER( kb_p2_w );
};


// device type definition
DECLARE_DEVICE_TYPE(TANDY2K_KEYBOARD, tandy2k_keyboard_device)

#endif // MAME_MACHINE_TANDY2KB_H
