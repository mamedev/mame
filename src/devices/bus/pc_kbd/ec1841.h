// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    EC-1841 92-key keyboard emulation

*********************************************************************/

#pragma once

#ifndef __PC_KBD_EC_1841__
#define __PC_KBD_EC_1841__

#include "cpu/mcs48/mcs48.h"
#include "pc_kbdc.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ec_1841_keyboard_device

class ec_1841_keyboard_device :  public device_t,
										public device_pc_kbd_interface
{
public:
	// construction/destruction
	ec_1841_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	DECLARE_WRITE8_MEMBER( bus_w );
	DECLARE_READ8_MEMBER( p1_r );
	DECLARE_WRITE8_MEMBER( p1_w );
	DECLARE_WRITE8_MEMBER( p2_w );
	DECLARE_READ_LINE_MEMBER( t1_r );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_pc_kbd_interface overrides
	virtual DECLARE_WRITE_LINE_MEMBER( clock_write ) override;
	virtual DECLARE_WRITE_LINE_MEMBER( data_write ) override;

private:
	required_device<cpu_device> m_maincpu;
	required_ioport_array<16> m_kbd;

	uint8_t m_bus;
	uint8_t m_p1;
	uint8_t m_p2;
	int m_q;
};


// device type definition
extern const device_type PC_KBD_EC_1841;

#endif
