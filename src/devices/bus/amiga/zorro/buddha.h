// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Buddha

    Zorro-II IDE controller

***************************************************************************/

#pragma once

#ifndef __BUDDHA_H__
#define __BUDDHA_H__

#include "emu.h"
#include "zorro.h"
#include "machine/autoconfig.h"
#include "machine/ataintf.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> buddha_device

class buddha_device : public device_t, public device_zorro2_card_interface, public amiga_autoconfig
{
public:
	// construction/destruction
	buddha_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// speed register
	uint16_t speed_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void speed_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	// ide register
	uint16_t ide_0_cs0_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void ide_0_cs0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t ide_0_cs1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void ide_0_cs1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t ide_1_cs0_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void ide_1_cs0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t ide_1_cs1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void ide_1_cs1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	// interrupt register
	uint16_t ide_0_interrupt_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t ide_1_interrupt_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void ide_interrupt_enable_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void ide_0_interrupt_w(int state);
	void ide_1_interrupt_w(int state);

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual void device_start() override;
	virtual void device_reset() override;

	// device_zorro2_card_interface overrides
	virtual void cfgin_w(int state) override;

	// amiga_autoconfig overrides
	virtual void autoconfig_base_address(offs_t address) override;

private:
	required_device<ata_interface_device> m_ata_0;
	required_device<ata_interface_device> m_ata_1;

	bool m_ide_interrupts_enabled;
	int m_ide_0_interrupt;
	int m_ide_1_interrupt;
};

// device type definition
extern const device_type BUDDHA;

#endif
