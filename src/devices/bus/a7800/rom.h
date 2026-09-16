// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_A7800_ROM_H
#define MAME_BUS_A7800_ROM_H

#pragma once

#include "a78_slot.h"
#include "sound/pokey.h"


// ======================> a78_rom_device

class a78_rom_device : public device_t,
						public device_a78_cart_interface
{
public:
	// construction/destruction
	a78_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_40xx(offs_t offset) override;

protected:
	a78_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};


// ======================> a78_rom_pokey_device

class a78_rom_pokey_device : public a78_rom_device
{
public:
	// construction/destruction
	a78_rom_pokey_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_40xx(offs_t offset) override;
	virtual void write_40xx(offs_t offset, uint8_t data) override;

protected:
	a78_rom_pokey_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<pokey_device> m_pokey;
};


// ======================> a78_rom_sg_ram_device

class a78_rom_mram_device : public a78_rom_device
{
public:
	// construction/destruction
	a78_rom_mram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_40xx(offs_t offset) override;
	virtual void write_40xx(offs_t offset, uint8_t data) override;

protected:
	a78_rom_mram_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> a78_rom_sg_device

class a78_rom_sg_device : public a78_rom_device
{
public:
	// construction/destruction
	a78_rom_sg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_40xx(offs_t offset) override;
	virtual void write_40xx(offs_t offset, uint8_t data) override;

protected:
	a78_rom_sg_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	int m_bank;
};


// ======================> a78_rom_sg_pokey_device

class a78_rom_sg_pokey_device : public a78_rom_sg_device
{
public:
	// construction/destruction
	a78_rom_sg_pokey_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_40xx(offs_t offset) override;
	virtual void write_40xx(offs_t offset, uint8_t data) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<pokey_device> m_pokey;
};


// ======================> a78_rom_sg_ram_device

class a78_rom_sg_ram_device : public a78_rom_sg_device
{
public:
	// construction/destruction
	a78_rom_sg_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_40xx(offs_t offset) override;
	virtual void write_40xx(offs_t offset, uint8_t data) override;

protected:
	a78_rom_sg_ram_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> a78_rom_sg9_device

class a78_rom_sg9_device : public a78_rom_sg_device
{
public:
	// construction/destruction
	a78_rom_sg9_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_40xx(offs_t offset) override;
	virtual void write_40xx(offs_t offset, uint8_t data) override;

protected:
	a78_rom_sg9_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> a78_rom_abs_device

class a78_rom_abs_device : public a78_rom_device
{
public:
	// construction/destruction
	a78_rom_abs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_40xx(offs_t offset) override;
	virtual void write_40xx(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	int m_bank;
};


// ======================> a78_rom_act_device

class a78_rom_act_device : public a78_rom_device
{
public:
	// construction/destruction
	a78_rom_act_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_40xx(offs_t offset) override;
	virtual void write_40xx(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	int m_bank;
};


// PCB variants with a POKEY at $0450

// ======================> a78_rom_p450_device

class a78_rom_p450_device : public a78_rom_device
{
public:
	// construction/destruction
	a78_rom_p450_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_04xx(offs_t offset) override { if (offset >= 0x50 && offset < 0x60) return m_pokey450->read(offset & 0x0f); else return 0xff; }
	virtual void write_04xx(offs_t offset, uint8_t data) override { if (offset >= 0x50 && offset < 0x60) m_pokey450->write(offset & 0x0f, data); }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<pokey_device> m_pokey450;
};


// ======================> a78_rom_p450_pokey_device

class a78_rom_p450_pokey_device : public a78_rom_pokey_device
{
public:
	// construction/destruction
	a78_rom_p450_pokey_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_04xx(offs_t offset) override { if (offset >= 0x50 && offset < 0x60) return m_pokey450->read(offset & 0x0f); else return 0xff; }
	virtual void write_04xx(offs_t offset, uint8_t data) override { if (offset >= 0x50 && offset < 0x60) m_pokey450->write(offset & 0x0f, data); }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<pokey_device> m_pokey450;
};


// ======================> a78_rom_p450_sg_ram_device

class a78_rom_p450_sg_ram_device : public a78_rom_sg_ram_device
{
public:
	// construction/destruction
	a78_rom_p450_sg_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_04xx(offs_t offset) override { if (offset >= 0x50 && offset < 0x60) return m_pokey450->read(offset & 0x0f); else return 0xff; }
	virtual void write_04xx(offs_t offset, uint8_t data) override { if (offset >= 0x50 && offset < 0x60) m_pokey450->write(offset & 0x0f, data); }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<pokey_device> m_pokey450;
};


// ======================> a78_rom_p450_sg9_device

class a78_rom_p450_sg9_device : public a78_rom_sg9_device
{
public:
	// construction/destruction
	a78_rom_p450_sg9_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_04xx(offs_t offset) override { if (offset >= 0x50 && offset < 0x60) return m_pokey450->read(offset & 0x0f); else return 0xff; }
	virtual void write_04xx(offs_t offset, uint8_t data) override { if (offset >= 0x50 && offset < 0x60) m_pokey450->write(offset & 0x0f, data); }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<pokey_device> m_pokey450;
};





// device type definition
DECLARE_DEVICE_TYPE(A78_ROM,             a78_rom_device)
DECLARE_DEVICE_TYPE(A78_ROM_SG,          a78_rom_sg_device)
DECLARE_DEVICE_TYPE(A78_ROM_POKEY,       a78_rom_pokey_device)
DECLARE_DEVICE_TYPE(A78_ROM_SG_POKEY,    a78_rom_sg_pokey_device)
DECLARE_DEVICE_TYPE(A78_ROM_SG_RAM,      a78_rom_sg_ram_device)
DECLARE_DEVICE_TYPE(A78_ROM_MRAM,        a78_rom_mram_device)
DECLARE_DEVICE_TYPE(A78_ROM_SG9,         a78_rom_sg9_device)
DECLARE_DEVICE_TYPE(A78_ROM_ABSOLUTE,    a78_rom_abs_device)
DECLARE_DEVICE_TYPE(A78_ROM_ACTIVISION,  a78_rom_act_device)

// PCB variants with a POKEY at $0450
DECLARE_DEVICE_TYPE(A78_ROM_P450,        a78_rom_p450_device)
DECLARE_DEVICE_TYPE(A78_ROM_P450_POKEY,  a78_rom_p450_pokey_device)
DECLARE_DEVICE_TYPE(A78_ROM_P450_SG_RAM, a78_rom_p450_sg_ram_device)
DECLARE_DEVICE_TYPE(A78_ROM_P450_SG9,    a78_rom_p450_sg9_device)

#endif // MAME_BUS_A7800_ROM_H
