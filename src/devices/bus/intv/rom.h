// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_INTV_ROM_H
#define MAME_BUS_INTV_ROM_H

#include "slot.h"


// ======================> intv_rom_device

class intv_rom_device : public device_t,
						public device_intv_cart_interface
{
public:
	// construction/destruction
	intv_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read_rom04(offs_t offset) override { return INTV_ROM16_READ(offset + 0x0400); }
	virtual uint16_t read_rom20(offs_t offset) override { return INTV_ROM16_READ(offset + 0x2000); }
	virtual uint16_t read_rom40(offs_t offset) override { return INTV_ROM16_READ(offset + 0x4000); }
	virtual uint16_t read_rom48(offs_t offset) override { return INTV_ROM16_READ(offset + 0x4800); }
	virtual uint16_t read_rom50(offs_t offset) override { return INTV_ROM16_READ(offset + 0x5000); }
	virtual uint16_t read_rom60(offs_t offset) override { return INTV_ROM16_READ(offset + 0x6000); }
	virtual uint16_t read_rom70(offs_t offset) override { return INTV_ROM16_READ(offset + 0x7000); }
	virtual uint16_t read_rom80(offs_t offset) override { return INTV_ROM16_READ(offset + 0x8000); }
	virtual uint16_t read_rom90(offs_t offset) override { return INTV_ROM16_READ(offset + 0x9000); }
	virtual uint16_t read_roma0(offs_t offset) override { return INTV_ROM16_READ(offset + 0xa000); }
	virtual uint16_t read_romb0(offs_t offset) override { return INTV_ROM16_READ(offset + 0xb000); }
	virtual uint16_t read_romc0(offs_t offset) override { return INTV_ROM16_READ(offset + 0xc000); }
	virtual uint16_t read_romd0(offs_t offset) override { return INTV_ROM16_READ(offset + 0xd000); }
	virtual uint16_t read_rome0(offs_t offset) override { return INTV_ROM16_READ(offset + 0xe000); }
	virtual uint16_t read_romf0(offs_t offset) override { return INTV_ROM16_READ(offset + 0xf000); }

protected:
	intv_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

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
	virtual uint16_t read_ram(offs_t offset) override { return (int)m_ram[offset & (m_ram.size() - 1)]; }
	virtual void write_ram(offs_t offset, uint16_t data) override { m_ram[offset & (m_ram.size() - 1)] = data & 0xff; }
};

// ======================> intv_gfact_device

class intv_gfact_device : public intv_rom_device
{
public:
	// construction/destruction
	intv_gfact_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read_ram(offs_t offset) override { return (int)m_ram[offset & (m_ram.size() - 1)]; }
	virtual void write_ram(offs_t offset, uint16_t data) override { m_ram[offset & (m_ram.size() - 1)] = data & 0xff; }
};

// ======================> intv_wsmlb_device

class intv_wsmlb_device : public intv_rom_device
{
public:
	// construction/destruction
	intv_wsmlb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};



// device type definition
DECLARE_DEVICE_TYPE(INTV_ROM_STD,   intv_rom_device)
DECLARE_DEVICE_TYPE(INTV_ROM_RAM,   intv_ram_device)
DECLARE_DEVICE_TYPE(INTV_ROM_GFACT, intv_gfact_device)
DECLARE_DEVICE_TYPE(INTV_ROM_WSMLB, intv_wsmlb_device)

#endif // MAME_BUS_INTV_ROM_H
