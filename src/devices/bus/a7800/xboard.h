// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_A7800_XBOARD_H
#define MAME_BUS_A7800_XBOARD_H

#pragma once

#include "a78_slot.h"
#include "rom.h"
#include "sound/pokey.h"
#include "sound/ymopm.h"


// ======================> a78_xboard_device

class a78_xboard_device : public a78_rom_device
{
public:
	// construction/destruction
	a78_xboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_04xx(offs_t offset) override;
	virtual void write_04xx(offs_t offset, uint8_t data) override;
	virtual uint8_t read_40xx(offs_t offset) override;
	virtual void write_40xx(offs_t offset, uint8_t data) override;

protected:
	a78_xboard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<a78_cart_slot_device> m_xbslot;
	required_device<pokey_device> m_pokey;
	int m_reg, m_ram_bank;
};


// ======================> a78_xm_device

class a78_xm_device : public a78_xboard_device
{
public:
	// construction/destruction
	a78_xm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_04xx(offs_t offset) override;
	virtual void write_04xx(offs_t offset, uint8_t data) override;
	virtual uint8_t read_10xx(offs_t offset) override;
	virtual void write_10xx(offs_t offset, uint8_t data) override;
	virtual uint8_t read_30xx(offs_t offset) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<ym2151_device> m_ym;
	int m_ym_enabled;
};



// device type definition
DECLARE_DEVICE_TYPE(A78_XBOARD, a78_xboard_device)
DECLARE_DEVICE_TYPE(A78_XM,     a78_xm_device)


#endif // MAME_BUS_A7800_XBOARD_H
