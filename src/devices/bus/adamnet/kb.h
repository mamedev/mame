// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Coleco Adam keyboard emulation

**********************************************************************/

#pragma once

#ifndef __ADAM_KB__
#define __ADAM_KB__

#include "emu.h"
#include "adamnet.h"
#include "cpu/m6800/m6800.h"



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

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	// not really public
	uint8_t p1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t p2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void p2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t p3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void p3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t p4_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void p4_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_adamnet_card_interface overrides
	virtual void adamnet_reset_w(int state) override;

	required_device<cpu_device> m_maincpu;
	required_ioport_array<13> m_y;

	uint16_t m_key_y;
};


// device type definition
extern const device_type ADAM_KB;



#endif
