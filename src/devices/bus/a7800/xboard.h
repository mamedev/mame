// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_A7800_XBOARD_H
#define MAME_BUS_A7800_XBOARD_H

#pragma once

#include "a78_slot.h"
#include "rom.h"
#include "sound/pokey.h"
#include "sound/ym2151.h"


// ======================> a78_xboard_device

class a78_xboard_device : public a78_rom_device
{
public:
	// construction/destruction
	a78_xboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_04xx) override;
	virtual DECLARE_WRITE8_MEMBER(write_04xx) override;
	virtual DECLARE_READ8_MEMBER(read_40xx) override;
	virtual DECLARE_WRITE8_MEMBER(write_40xx) override;

protected:
	a78_xboard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

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
	virtual DECLARE_READ8_MEMBER(read_04xx) override;
	virtual DECLARE_WRITE8_MEMBER(write_04xx) override;
	virtual DECLARE_READ8_MEMBER(read_10xx) override;
	virtual DECLARE_WRITE8_MEMBER(write_10xx) override;
	virtual DECLARE_READ8_MEMBER(read_30xx) override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	required_device<ym2151_device> m_ym;
	int m_ym_enabled;
};



// device type definition
DECLARE_DEVICE_TYPE(A78_XBOARD, a78_xboard_device)
DECLARE_DEVICE_TYPE(A78_XM,     a78_xm_device)


#endif // MAME_BUS_A7800_XBOARD_H
