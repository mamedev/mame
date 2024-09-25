// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy 2000 keyboard emulation

*********************************************************************/

#ifndef MAME_TRS_TANDY2KB_H
#define MAME_TRS_TANDY2KB_H

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

	void power_w(int state);
	void reset_w(int state);
	void busy_w(int state);
	int data_r();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

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

#endif // MAME_TRS_TANDY2KB_H
