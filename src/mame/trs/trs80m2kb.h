// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy Radio Shack TRS-80 Model II keyboard emulation

**********************************************************************/

#ifndef MAME_TRS_TRS80M2KB_H
#define MAME_TRS_TRS80M2KB_H

#pragma once

#include "cpu/mcs48/mcs48.h"
#include "sound/discrete.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define TRS80M2_KEYBOARD_TAG    "trs80m2kb"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> trs80m2_keyboard_device

class trs80m2_keyboard_device :  public device_t
{
public:
	// construction/destruction
	trs80m2_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto clock_wr_callback() { return m_write_clock.bind(); }

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
		LED_0 = 0,
		LED_1
	};

	required_device<i8021_device> m_maincpu;
	required_ioport_array<12> m_y;
	output_finder<2> m_leds;

	devcb_write_line   m_write_clock;

	int m_busy;
	int m_data;
	int m_clk;

	uint8_t m_keylatch;

	int kb_t1_r();
	uint8_t kb_p0_r();
	void kb_p1_w(uint8_t data);
	void kb_p2_w(uint8_t data);
};


// device type definition
DECLARE_DEVICE_TYPE(TRS80M2_KEYBOARD, trs80m2_keyboard_device)



#endif // MAME_TRS_TRS80M2KB_H
