// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Angelo Salese
#ifndef MAME_BUS_A800_OSS_H
#define MAME_BUS_A800_OSS_H

#pragma once

#include "rom.h"
#include "machine/bankdev.h"


class a800_rom_oss8k_device : public a800_rom_device
{
public:
	a800_rom_oss8k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	a800_rom_oss8k_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
	virtual void cctl_map(address_map &map) override ATTR_COLD;
	virtual std::tuple<int, int> get_initial_rd_state() override { return std::make_tuple(0, 1); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	int m_bank;
	virtual void bank_config_access(offs_t offset);
	u8 rom_bank_r(offs_t offset);
	void rom_bank_w(offs_t offset, u8 data);
};


class a800_rom_oss043m_device : public a800_rom_device
{
public:
	a800_rom_oss043m_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	a800_rom_oss043m_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
	virtual void cctl_map(address_map &map) override ATTR_COLD;
	virtual std::tuple<int, int> get_initial_rd_state() override { return std::make_tuple(0, 1); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void bankdev_map(address_map &map) ATTR_COLD;
	void bank_config_access(offs_t offset);
	u8 rom_bank_r(offs_t offset);
	void rom_bank_w(offs_t offset, u8 data);

	required_device<address_map_bank_device> m_bankdev;

	u16 m_bank_base1, m_bank_base2;
};


class a800_rom_oss034m_device : public a800_rom_oss043m_device
{
public:
	a800_rom_oss034m_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};


class a800_rom_oss091m_device : public a800_rom_oss8k_device
{
public:
	a800_rom_oss091m_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void bank_config_access(offs_t offset) override;
};



DECLARE_DEVICE_TYPE(A800_ROM_OSS8K, a800_rom_oss8k_device)
DECLARE_DEVICE_TYPE(A800_ROM_OSS34, a800_rom_oss034m_device)
DECLARE_DEVICE_TYPE(A800_ROM_OSS43, a800_rom_oss043m_device)
DECLARE_DEVICE_TYPE(A800_ROM_OSS91, a800_rom_oss091m_device)


#endif // MAME_BUS_A800_OSS_H
