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
	a78_versaboard_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);
	a78_versaboard_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_40xx) override;
	virtual DECLARE_WRITE8_MEMBER(write_40xx) override;

protected:
	int m_ram_bank;
};


// ======================> a78_megacart_device

class a78_megacart_device : public a78_versaboard_device
{
public:
	// construction/destruction
	a78_megacart_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// reading and writing
	virtual DECLARE_WRITE8_MEMBER(write_40xx) override;
};


// VersaBoard variants of the standard carts + POKEY at 0x0450!

// ======================> a78_rom_p450_vb_device

class a78_rom_p450_vb_device : public a78_versaboard_device
{
public:
	// construction/destruction
	a78_rom_p450_vb_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_04xx) override { if (offset >= 0x50 && offset < 0x60) return m_pokey450->read(space, offset & 0x0f); else return 0xff; }
	virtual DECLARE_WRITE8_MEMBER(write_04xx) override { if (offset >= 0x50 && offset < 0x60) m_pokey450->write(space, offset & 0x0f, data); }

protected:
	required_device<pokey_device> m_pokey450;
};



// device type definition
extern const device_type A78_ROM_VERSABOARD;
extern const device_type A78_ROM_MEGACART;

extern const device_type A78_ROM_P450_VB;

#endif
