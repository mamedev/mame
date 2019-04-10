// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Coleco Adam keyboard emulation

**********************************************************************/

#ifndef MAME_BUS_ADAMNET_KB_H
#define MAME_BUS_ADAMNET_KB_H

#pragma once

#include "adamnet.h"
#include "cpu/m6800/m6801.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> adam_keyboard_device

class adam_keyboard_device :  public device_t,
								public device_adamnet_card_interface
{
public:
	// construction/destruction
	adam_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	// device_adamnet_card_interface overrides
	virtual void adamnet_reset_w(int state) override;

private:
	required_device<m6801_cpu_device> m_maincpu;
	required_ioport_array<13> m_y;

	uint16_t m_key_y;

	DECLARE_READ8_MEMBER( p1_r );
	DECLARE_READ8_MEMBER( p2_r );
	DECLARE_WRITE8_MEMBER( p2_w );
	DECLARE_READ8_MEMBER( p3_r );
	DECLARE_WRITE8_MEMBER( p3_w );
	DECLARE_READ8_MEMBER( p4_r );
	DECLARE_WRITE8_MEMBER( p4_w );

	void adam_kb_mem(address_map &map);
};


// device type definition
DECLARE_DEVICE_TYPE(ADAM_KB, adam_keyboard_device)



#endif // MAME_BUS_ADAMNET_KB_H
