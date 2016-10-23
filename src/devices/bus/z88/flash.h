// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#pragma once

#ifndef __Z88_FLASH_H__
#define __Z88_FLASH_H__

#include "emu.h"
#include "z88.h"
#include "machine/intelfsh.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> z88_1024k_flash_device

class z88_1024k_flash_device : public device_t,
								public device_z88cart_interface
{
public:
	// construction/destruction
	z88_1024k_flash_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	// device-level overrides
	virtual void device_start() override;

	// z88cart_interface overrides
	virtual uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual uint8_t* get_cart_base() override;
	virtual uint32_t get_cart_size() override { return 0x100000; }

private:
	required_device<intelfsh8_device> m_flash;
};

// device type definition
extern const device_type Z88_1024K_FLASH;


#endif  /* __Z88_FLASH_H__ */
