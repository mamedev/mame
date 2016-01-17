// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/**********************************************************************

    Decision Mate V keyboard emulation

*********************************************************************/

#pragma once

#ifndef __DMV_KEYBOARD__
#define __DMV_KEYBOARD__


#include "emu.h"
#include "cpu/mcs48/mcs48.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_DMV_KEYBOARD_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, DMV_KEYBOARD, 0)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dmv_keyboard_device

class dmv_keyboard_device : public device_t
{
public:
	// construction/destruction
	dmv_keyboard_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	DECLARE_WRITE_LINE_MEMBER(sd_poll_w);
	DECLARE_READ_LINE_MEMBER(sd_poll_r);

	DECLARE_READ8_MEMBER( port1_r );
	DECLARE_READ8_MEMBER( port2_r );
	DECLARE_WRITE8_MEMBER( port2_w );

protected:
	// device-level overrides
	virtual void device_config_complete() override { m_shortname = "dmv_keyb"; }
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<upi41_cpu_device> m_maincpu;
	required_ioport_array<16> m_keyboard;

	UINT8   m_col;
	int     m_sd_data_state;
	int     m_sd_poll_state;
};


// device type definition
extern const device_type DMV_KEYBOARD;


#endif
