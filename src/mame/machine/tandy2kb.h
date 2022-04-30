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
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tandy2k_keyboard_device

class tandy2k_keyboard_device :  public device_t
{
public:
	// construction/destruction
	tandy2k_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto clock_wr_callback() { return m_write_clock.bind(); }
	auto data_wr_callback() { return m_write_data.bind(); }

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

	required_device<i8048_device> m_maincpu;
	required_ioport_array<12> m_y;
	output_finder<2> m_leds;

	devcb_write_line   m_write_clock;
	devcb_write_line   m_write_data;

	uint16_t m_keylatch;

	int m_clock;
	int m_data;

	uint8_t kb_p1_r();
	void kb_p1_w(uint8_t data);
	void kb_p2_w(uint8_t data);
};


// device type definition
DECLARE_DEVICE_TYPE(TANDY2K_KEYBOARD, tandy2k_keyboard_device)

#endif // MAME_MACHINE_TANDY2KB_H
