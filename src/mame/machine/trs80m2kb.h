// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy Radio Shack TRS-80 Model II keyboard emulation

**********************************************************************/

#pragma once

#ifndef __TRS80M2_KEYBOARD__
#define __TRS80M2_KEYBOARD__

#include "emu.h"
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
	trs80m2_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_clock_wr_callback(device_t &device, _Object object) { return downcast<trs80m2_keyboard_device &>(device).m_write_clock.set_callback(object); }

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	DECLARE_WRITE_LINE_MEMBER( busy_w );
	DECLARE_READ_LINE_MEMBER( data_r );

	// not really public
	DECLARE_READ8_MEMBER( kb_t1_r );
	DECLARE_READ8_MEMBER( kb_p0_r );
	DECLARE_WRITE8_MEMBER( kb_p1_w );
	DECLARE_WRITE8_MEMBER( kb_p2_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:
	enum
	{
		LED_0 = 0,
		LED_1
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
	required_ioport m_ya;
	required_ioport m_yb;

	devcb_write_line   m_write_clock;

	int m_busy;
	int m_data;
	int m_clk;

	UINT8 m_y;
};


// device type definition
extern const device_type TRS80M2_KEYBOARD;



#endif
