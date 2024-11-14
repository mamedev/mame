// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_SNES_SFX_H
#define MAME_BUS_SNES_SFX_H

#include "snes_slot.h"
#include "rom.h"
#include "cpu/superfx/superfx.h"


// ======================> sns_rom_superfx_device

class sns_rom_superfx_device : public sns_rom_device
{
protected:
	// construction/destruction
	sns_rom_superfx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void snes_extern_irq_w(int state);

	// additional reading and writing
	virtual uint8_t read_l(offs_t offset) override;
	virtual uint8_t read_h(offs_t offset) override;
	virtual uint8_t read_ram(offs_t offset) override;
	virtual void write_ram(offs_t offset, uint8_t data) override;
	virtual uint8_t chip_read(offs_t offset) override;
	virtual void chip_write(offs_t offset, uint8_t data) override;

	uint8_t superfx_r_bank1(offs_t offset);
	uint8_t superfx_r_bank2(offs_t offset);
	uint8_t superfx_r_bank3(offs_t offset);
	void superfx_w_bank1(offs_t offset, uint8_t data);
	void superfx_w_bank2(offs_t offset, uint8_t data);
	void superfx_w_bank3(offs_t offset, uint8_t data);

	required_device<superfx_device> m_superfx;

	uint8_t sfx_ram[0x200000];

	void sfx_map(address_map &map) ATTR_COLD;
};

class sns_rom_superfx1_device : public sns_rom_superfx_device
{
public:
	// construction/destruction
	sns_rom_superfx1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class sns_rom_superfx2_device : public sns_rom_superfx_device
{
public:
	// construction/destruction
	sns_rom_superfx2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

// device type definition
DECLARE_DEVICE_TYPE(SNS_LOROM_SUPERFX1, sns_rom_superfx1_device)
DECLARE_DEVICE_TYPE(SNS_LOROM_SUPERFX2, sns_rom_superfx2_device)

#endif // MAME_BUS_SNES_SFX_H
