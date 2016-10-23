// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __SNS_SUFAMI_H
#define __SNS_SUFAMI_H

#include "snes_slot.h"
#include "rom.h"


// ======================> sns_rom_sufami_device

class sns_rom_sufami_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_sufami_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// additional reading and writing
	virtual uint8_t read_l(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_h(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_l(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

private:
	required_device<sns_sufami_cart_slot_device> m_slot1;
	required_device<sns_sufami_cart_slot_device> m_slot2;
};

// ======================> sns_rom_strom_device

class sns_rom_strom_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_strom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	// additional reading and writing
	virtual uint8_t read_l(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
};


// device type definition
extern const device_type SNS_LOROM_SUFAMI;
extern const device_type SNS_STROM;

#endif
