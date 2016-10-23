// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __A78_XBOARD_H
#define __A78_XBOARD_H

#include "a78_slot.h"
#include "rom.h"
#include "sound/pokey.h"
#include "sound/ym2151.h"


// ======================> a78_xboard_device

class a78_xboard_device : public a78_rom_device
{
public:
	// construction/destruction
	a78_xboard_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	a78_xboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_reset() override;

	// reading and writing
	virtual uint8_t read_04xx(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_04xx(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_40xx(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_40xx(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

protected:
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

	// device-level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_reset() override;

	// reading and writing
	virtual uint8_t read_04xx(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_04xx(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_10xx(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_10xx(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_30xx(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;

protected:
	required_device<ym2151_device> m_ym;
	int m_ym_enabled;
};



// device type definition
extern const device_type A78_XBOARD;
extern const device_type A78_XM;


#endif
