// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __A78_HISCORE_H
#define __A78_HISCORE_H

#include "a78_slot.h"
#include "rom.h"


// ======================> a78_hiscore_device

class a78_hiscore_device : public a78_rom_device
{
public:
	// construction/destruction
	a78_hiscore_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
	virtual uint8_t read_04xx(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_04xx(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_10xx(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_10xx(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_30xx(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_40xx(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_40xx(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

protected:
	required_device<a78_cart_slot_device> m_hscslot;
};



// device type definition
extern const device_type A78_HISCORE;


#endif
