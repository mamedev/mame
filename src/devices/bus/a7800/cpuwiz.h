// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __A78_CPUWIZ_H
#define __A78_CPUWIZ_H

#include "a78_slot.h"
#include "rom.h"


// ======================> a78_versaboard_device

class a78_versaboard_device : public a78_rom_sg_device
{
public:
	// construction/destruction
	a78_versaboard_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	a78_versaboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual uint8_t read_40xx(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_40xx(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

protected:
	int m_ram_bank;
};


// ======================> a78_megacart_device

class a78_megacart_device : public a78_versaboard_device
{
public:
	// construction/destruction
	a78_megacart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual void write_40xx(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
};


// VersaBoard variants of the standard carts + POKEY at 0x0450!

// ======================> a78_rom_p450_vb_device

class a78_rom_p450_vb_device : public a78_versaboard_device
{
public:
	// construction/destruction
	a78_rom_p450_vb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
	virtual uint8_t read_04xx(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override { if (offset >= 0x50 && offset < 0x60) return m_pokey450->read(space, offset & 0x0f); else return 0xff; }
	virtual void write_04xx(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override { if (offset >= 0x50 && offset < 0x60) m_pokey450->write(space, offset & 0x0f, data); }

protected:
	required_device<pokey_device> m_pokey450;
};



// device type definition
extern const device_type A78_ROM_VERSABOARD;
extern const device_type A78_ROM_MEGACART;

extern const device_type A78_ROM_P450_VB;

#endif
