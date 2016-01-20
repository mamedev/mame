// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Visual 1050 keyboard emulation

*********************************************************************/

#pragma once

#ifndef __V1050_KEYBOARD__
#define __V1050_KEYBOARD__

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "sound/discrete.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_V1050_KEYBOARD_OUT_TX_HANDLER(_devcb) \
	devcb = &v1050_keyboard_device::set_out_tx_handler(*device, DEVCB_##_devcb);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> V1050_keyboard_device

class v1050_keyboard_device :  public device_t
{
public:
	// construction/destruction
	v1050_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_out_tx_handler(device_t &device, _Object object) { return downcast<v1050_keyboard_device &>(device).m_out_tx_handler.set_callback(object); }

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	DECLARE_WRITE_LINE_MEMBER( si_w );

	// not really public
	DECLARE_READ8_MEMBER( kb_p1_r );
	DECLARE_WRITE8_MEMBER( kb_p1_w );
	DECLARE_WRITE8_MEMBER( kb_p2_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<discrete_sound_device> m_discrete;
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
	devcb_write_line   m_out_tx_handler;

	UINT8 m_y;
};


// device type definition
extern const device_type V1050_KEYBOARD;



#endif
