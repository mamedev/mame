// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_A800_SIC_H
#define MAME_BUS_A800_SIC_H

#pragma once

#include "rom.h"
#include "machine/intelfsh.h"

class a800_sic_128kb_device : public a800_rom_device
{
public:
	a800_sic_128kb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	a800_sic_128kb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
	virtual void cctl_map(address_map &map) override ATTR_COLD;
	virtual std::tuple<int, int> get_initial_rd_state() override { return std::make_tuple(0, 1); }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	required_device<intelfsh8_device> m_flash;

private:
	int m_bank;
	bool m_write_protect;
	int m_bank_mask;

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	u8 config_bank_r(offs_t offset);
	void config_bank_w(offs_t offset, u8 data);
};

class a800_sic_256kb_device : public a800_sic_128kb_device
{
public:
	a800_sic_256kb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class a800_sic_512kb_device : public a800_sic_128kb_device
{
public:
	a800_sic_512kb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(A800_SIC_128KB, a800_sic_128kb_device)
DECLARE_DEVICE_TYPE(A800_SIC_256KB, a800_sic_256kb_device)
DECLARE_DEVICE_TYPE(A800_SIC_512KB, a800_sic_512kb_device)

#endif // MAME_BUS_A800_SIC_H
