// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy Radio Shack TRS-80 Model II keyboard emulation

**********************************************************************/

#ifndef MAME_MACHINE_TRS80M2KB_H
#define MAME_MACHINE_TRS80M2KB_H

#pragma once

#include "cpu/mcs48/mcs48.h"
#include "sound/discrete.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define TRS80M2_KEYBOARD_TAG    "trs80m2kb"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_TRS80M2_KEYBOARD_CLOCK_CALLBACK(_write) \
	devcb = &trs80m2_keyboard_device::set_clock_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> trs80m2_keyboard_device

class trs80m2_keyboard_device :  public device_t
{
public:
	// construction/destruction
	trs80m2_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> static devcb_base &set_clock_wr_callback(device_t &device, Object &&cb) { return downcast<trs80m2_keyboard_device &>(device).m_write_clock.set_callback(std::forward<Object>(cb)); }

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	DECLARE_WRITE_LINE_MEMBER( busy_w );
	DECLARE_READ_LINE_MEMBER( data_r );

	// not really public
	DECLARE_READ_LINE_MEMBER( kb_t1_r );
	DECLARE_READ8_MEMBER( kb_p0_r );
	DECLARE_WRITE8_MEMBER( kb_p1_w );
	DECLARE_WRITE8_MEMBER( kb_p2_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	enum
	{
		LED_0 = 0,
		LED_1
	};

	required_device<cpu_device> m_maincpu;
	required_ioport_array<12> m_y;

	devcb_write_line   m_write_clock;

	int m_busy;
	int m_data;
	int m_clk;

	uint8_t m_keylatch;
};


// device type definition
DECLARE_DEVICE_TYPE(TRS80M2_KEYBOARD, trs80m2_keyboard_device)



#endif // MAME_MACHINE_TRS80M2KB_H
