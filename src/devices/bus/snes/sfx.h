// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __SNS_SFX_H
#define __SNS_SFX_H

#include "snes_slot.h"
#include "rom.h"
#include "cpu/superfx/superfx.h"


// ======================> sns_rom_superfx_device

class sns_rom_superfx_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_superfx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	required_device<superfx_device> m_superfx;

	// additional reading and writing
	virtual uint8_t read_l(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_h(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_ram(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual uint8_t chip_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void chip_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual uint8_t superfx_r_bank1(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	virtual uint8_t superfx_r_bank2(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	virtual uint8_t superfx_r_bank3(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	virtual void superfx_w_bank1(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void superfx_w_bank2(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void superfx_w_bank3(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void snes_extern_irq_w(int state);


	uint8_t sfx_ram[0x200000];
};


// device type definition
extern const device_type SNS_LOROM_SUPERFX;

#endif
