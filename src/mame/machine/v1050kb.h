// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Visual 1050 keyboard emulation

*********************************************************************/

#pragma once

#ifndef __V1050_KEYBOARD__
#define __V1050_KEYBOARD__

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
	v1050_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _Object> static devcb_base &set_out_tx_handler(device_t &device, _Object object) { return downcast<v1050_keyboard_device &>(device).m_out_tx_handler.set_callback(object); }

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
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
	required_ioport_array<12> m_y;
	devcb_write_line   m_out_tx_handler;

	uint8_t m_keylatch;
};


// device type definition
extern const device_type V1050_KEYBOARD;



#endif
