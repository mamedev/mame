// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __INTV_ROM_H
#define __INTV_ROM_H

#include "slot.h"


// ======================> intv_rom_device

class intv_rom_device : public device_t,
						public device_intv_cart_interface
{
public:
	// construction/destruction
	intv_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	intv_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read_rom04(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return INTV_ROM16_READ(offset + 0x0400); }
	virtual uint16_t read_rom20(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return INTV_ROM16_READ(offset + 0x2000); }
	virtual uint16_t read_rom40(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return INTV_ROM16_READ(offset + 0x4000); }
	virtual uint16_t read_rom48(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return INTV_ROM16_READ(offset + 0x4800); }
	virtual uint16_t read_rom50(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return INTV_ROM16_READ(offset + 0x5000); }
	virtual uint16_t read_rom60(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return INTV_ROM16_READ(offset + 0x6000); }
	virtual uint16_t read_rom70(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return INTV_ROM16_READ(offset + 0x7000); }
	virtual uint16_t read_rom80(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return INTV_ROM16_READ(offset + 0x8000); }
	virtual uint16_t read_rom90(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return INTV_ROM16_READ(offset + 0x9000); }
	virtual uint16_t read_roma0(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return INTV_ROM16_READ(offset + 0xa000); }
	virtual uint16_t read_romb0(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return INTV_ROM16_READ(offset + 0xb000); }
	virtual uint16_t read_romc0(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return INTV_ROM16_READ(offset + 0xc000); }
	virtual uint16_t read_romd0(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return INTV_ROM16_READ(offset + 0xd000); }
	virtual uint16_t read_rome0(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return INTV_ROM16_READ(offset + 0xe000); }
	virtual uint16_t read_romf0(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return INTV_ROM16_READ(offset + 0xf000); }

	// device-level overrides
	virtual void device_start() override {}
	virtual void device_reset() override {}
};

// ======================> intv_ram_device

class intv_ram_device : public intv_rom_device
{
public:
	// construction/destruction
	intv_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read_ram(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return (int)m_ram[offset & (m_ram.size() - 1)]; }
	virtual void write_ram(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) override { m_ram[offset & (m_ram.size() - 1)] = data & 0xff; }
};

// ======================> intv_gfact_device

class intv_gfact_device : public intv_rom_device
{
public:
	// construction/destruction
	intv_gfact_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read_ram(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return (int)m_ram[offset & (m_ram.size() - 1)]; }
	virtual void write_ram(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) override { m_ram[offset & (m_ram.size() - 1)] = data & 0xff; }
};

// ======================> intv_wsmlb_device

class intv_wsmlb_device : public intv_rom_device
{
public:
	// construction/destruction
	intv_wsmlb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};



// device type definition
extern const device_type INTV_ROM_STD;
extern const device_type INTV_ROM_RAM;
extern const device_type INTV_ROM_GFACT;
extern const device_type INTV_ROM_WSMLB;

#endif
