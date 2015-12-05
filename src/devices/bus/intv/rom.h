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
	intv_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	intv_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read_rom04) { return INTV_ROM16_READ(offset + 0x0400); }
	virtual DECLARE_READ16_MEMBER(read_rom20) { return INTV_ROM16_READ(offset + 0x2000); }
	virtual DECLARE_READ16_MEMBER(read_rom40) { return INTV_ROM16_READ(offset + 0x4000); }
	virtual DECLARE_READ16_MEMBER(read_rom48) { return INTV_ROM16_READ(offset + 0x4800); }
	virtual DECLARE_READ16_MEMBER(read_rom50) { return INTV_ROM16_READ(offset + 0x5000); }
	virtual DECLARE_READ16_MEMBER(read_rom60) { return INTV_ROM16_READ(offset + 0x6000); }
	virtual DECLARE_READ16_MEMBER(read_rom70) { return INTV_ROM16_READ(offset + 0x7000); }
	virtual DECLARE_READ16_MEMBER(read_rom80) { return INTV_ROM16_READ(offset + 0x8000); }
	virtual DECLARE_READ16_MEMBER(read_rom90) { return INTV_ROM16_READ(offset + 0x9000); }
	virtual DECLARE_READ16_MEMBER(read_roma0) { return INTV_ROM16_READ(offset + 0xa000); }
	virtual DECLARE_READ16_MEMBER(read_romb0) { return INTV_ROM16_READ(offset + 0xb000); }
	virtual DECLARE_READ16_MEMBER(read_romc0) { return INTV_ROM16_READ(offset + 0xc000); }
	virtual DECLARE_READ16_MEMBER(read_romd0) { return INTV_ROM16_READ(offset + 0xd000); }
	virtual DECLARE_READ16_MEMBER(read_rome0) { return INTV_ROM16_READ(offset + 0xe000); }
	virtual DECLARE_READ16_MEMBER(read_romf0) { return INTV_ROM16_READ(offset + 0xf000); }

	// device-level overrides
	virtual void device_start() {}
	virtual void device_reset() {}
};

// ======================> intv_ram_device

class intv_ram_device : public intv_rom_device
{
public:
	// construction/destruction
	intv_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read_ram) { return (int)m_ram[offset & (m_ram.size() - 1)]; }
	virtual DECLARE_WRITE16_MEMBER(write_ram) { m_ram[offset & (m_ram.size() - 1)] = data & 0xff; }
};

// ======================> intv_gfact_device

class intv_gfact_device : public intv_rom_device
{
public:
	// construction/destruction
	intv_gfact_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read_ram) { return (int)m_ram[offset & (m_ram.size() - 1)]; }
	virtual DECLARE_WRITE16_MEMBER(write_ram) { m_ram[offset & (m_ram.size() - 1)] = data & 0xff; }
};

// ======================> intv_wsmlb_device

class intv_wsmlb_device : public intv_rom_device
{
public:
	// construction/destruction
	intv_wsmlb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};



// device type definition
extern const device_type INTV_ROM_STD;
extern const device_type INTV_ROM_RAM;
extern const device_type INTV_ROM_GFACT;
extern const device_type INTV_ROM_WSMLB;

#endif
