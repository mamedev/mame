// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Victor 9000 keyboard emulation

*********************************************************************/

#pragma once

#ifndef __VICTOR9K_KEYBOARD__
#define __VICTOR9K_KEYBOARD__


#include "emu.h"
#include "cpu/mcs48/mcs48.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_VICTOR9K_KBRDY_HANDLER(_devcb) \
	devcb = &victor_9000_keyboard_t::set_kbrdy_cb(*device, DEVCB_##_devcb);

#define MCFG_VICTOR9K_KBDATA_HANDLER(_devcb) \
	devcb = &victor_9000_keyboard_t::set_kbdata_cb(*device, DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> victor_9000_keyboard_t

class victor_9000_keyboard_t :  public device_t
{
public:
	// construction/destruction
	victor_9000_keyboard_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_kbrdy_cb(device_t &device, _Object object) { return downcast<victor_9000_keyboard_t &>(device).m_kbrdy_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_kbdata_cb(device_t &device, _Object object) { return downcast<victor_9000_keyboard_t &>(device).m_kbdata_cb.set_callback(object); }

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	DECLARE_WRITE_LINE_MEMBER( kback_w );

	// not really public
	DECLARE_READ8_MEMBER( kb_p1_r );
	DECLARE_WRITE8_MEMBER( kb_p1_w );
	DECLARE_WRITE8_MEMBER( kb_p2_w );
	DECLARE_READ8_MEMBER( kb_t1_r );

protected:
	// device-level overrides
	virtual void device_start() override;

private:
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
	required_ioport m_yc;

	devcb_write_line   m_kbrdy_cb;
	devcb_write_line   m_kbdata_cb;

	UINT8 m_p1;
	UINT8 m_y;
	int m_stb;
	int m_y12;
	int m_kbrdy;
	int m_kbdata;
	int m_kback;
};


// device type definition
extern const device_type VICTOR9K_KEYBOARD;



#endif
