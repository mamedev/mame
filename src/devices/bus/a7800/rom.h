// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __A78_ROM_H
#define __A78_ROM_H

#include "a78_slot.h"
#include "sound/pokey.h"


// ======================> a78_rom_device

class a78_rom_device : public device_t,
						public device_a78_cart_interface
{
public:
	// construction/destruction
	a78_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	a78_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_40xx);
};


// ======================> a78_rom_pokey_device

class a78_rom_pokey_device : public a78_rom_device
{
public:
	// construction/destruction
	a78_rom_pokey_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	a78_rom_pokey_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_40xx);
	virtual DECLARE_WRITE8_MEMBER(write_40xx);

protected:
	required_device<pokey_device> m_pokey;
};


// ======================> a78_rom_sg_device

class a78_rom_sg_device : public a78_rom_device
{
public:
	// construction/destruction
	a78_rom_sg_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	a78_rom_sg_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_40xx);
	virtual DECLARE_WRITE8_MEMBER(write_40xx);

protected:
	int m_bank;
};


// ======================> a78_rom_sg_pokey_device

class a78_rom_sg_pokey_device : public a78_rom_sg_device
{
public:
	// construction/destruction
	a78_rom_sg_pokey_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_40xx);
	virtual DECLARE_WRITE8_MEMBER(write_40xx);

protected:
	required_device<pokey_device> m_pokey;
};


// ======================> a78_rom_sg_ram_device

class a78_rom_sg_ram_device : public a78_rom_sg_device
{
public:
	// construction/destruction
	a78_rom_sg_ram_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	a78_rom_sg_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_40xx);
	virtual DECLARE_WRITE8_MEMBER(write_40xx);
};


// ======================> a78_rom_sg9_device

class a78_rom_sg9_device : public a78_rom_sg_device
{
public:
	// construction/destruction
	a78_rom_sg9_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	a78_rom_sg9_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_40xx);
	virtual DECLARE_WRITE8_MEMBER(write_40xx);
};


// ======================> a78_rom_abs_device

class a78_rom_abs_device : public a78_rom_device
{
public:
	// construction/destruction
	a78_rom_abs_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_40xx);
	virtual DECLARE_WRITE8_MEMBER(write_40xx);

protected:
	int m_bank;
};


// ======================> a78_rom_act_device

class a78_rom_act_device : public a78_rom_device
{
public:
	// construction/destruction
	a78_rom_act_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_40xx);
	virtual DECLARE_WRITE8_MEMBER(write_40xx);

protected:
	int m_bank;
};


// PCB variants with a POKEY at $0450

// ======================> a78_rom_p450_device

class a78_rom_p450_device : public a78_rom_device
{
public:
	// construction/destruction
	a78_rom_p450_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_04xx) override { if (offset >= 0x50 && offset < 0x60) return m_pokey450->read(space, offset & 0x0f); else return 0xff; }
	virtual DECLARE_WRITE8_MEMBER(write_04xx) override { if (offset >= 0x50 && offset < 0x60) m_pokey450->write(space, offset & 0x0f, data); }

protected:
	required_device<pokey_device> m_pokey450;
};


// ======================> a78_rom_p450_pokey_device

class a78_rom_p450_pokey_device : public a78_rom_pokey_device
{
public:
	// construction/destruction
	a78_rom_p450_pokey_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_04xx) override { if (offset >= 0x50 && offset < 0x60) return m_pokey450->read(space, offset & 0x0f); else return 0xff; }
	virtual DECLARE_WRITE8_MEMBER(write_04xx) override { if (offset >= 0x50 && offset < 0x60) m_pokey450->write(space, offset & 0x0f, data); }

protected:
	required_device<pokey_device> m_pokey450;
};


// ======================> a78_rom_p450_sg_ram_device

class a78_rom_p450_sg_ram_device : public a78_rom_sg_ram_device
{
public:
	// construction/destruction
	a78_rom_p450_sg_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_04xx) override { if (offset >= 0x50 && offset < 0x60) return m_pokey450->read(space, offset & 0x0f); else return 0xff; }
	virtual DECLARE_WRITE8_MEMBER(write_04xx) override { if (offset >= 0x50 && offset < 0x60) m_pokey450->write(space, offset & 0x0f, data); }

protected:
	required_device<pokey_device> m_pokey450;
};


// ======================> a78_rom_p450_sg9_device

class a78_rom_p450_sg9_device : public a78_rom_sg9_device
{
public:
	// construction/destruction
	a78_rom_p450_sg9_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_04xx) override { if (offset >= 0x50 && offset < 0x60) return m_pokey450->read(space, offset & 0x0f); else return 0xff; }
	virtual DECLARE_WRITE8_MEMBER(write_04xx) override { if (offset >= 0x50 && offset < 0x60) m_pokey450->write(space, offset & 0x0f, data); }

protected:
	required_device<pokey_device> m_pokey450;
};





// device type definition
extern const device_type A78_ROM;
extern const device_type A78_ROM_SG;
extern const device_type A78_ROM_POKEY;
extern const device_type A78_ROM_SG_POKEY;
extern const device_type A78_ROM_SG_RAM;
extern const device_type A78_ROM_SG9;
extern const device_type A78_ROM_ABSOLUTE;
extern const device_type A78_ROM_ACTIVISION;

// PCB variants with a POKEY at $0450
extern const device_type A78_ROM_P450;
extern const device_type A78_ROM_P450_POKEY;
extern const device_type A78_ROM_P450_SG_RAM;
extern const device_type A78_ROM_P450_SG9;

#endif
