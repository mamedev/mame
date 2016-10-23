// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef __VCS_HARMONY_H
#define __VCS_HARMONY_H

#include "rom.h"
#include "cpu/arm7/lpc210x.h"


// ======================> a26_rom_harmony_device

class a26_rom_harmony_device : public a26_rom_f8_device
{
public:
	// construction/destruction
	a26_rom_harmony_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_reset() override;

	// reading and writing
	virtual uint8_t read_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_bank(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;


	uint8_t read8_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void check_bankswitch(offs_t offset);

private:
	required_device<lpc210x_device> m_cpu;
};


// device type definition
extern const device_type A26_ROM_HARMONY;

#endif
