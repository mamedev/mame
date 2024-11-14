// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_A800_CORINA_H
#define MAME_BUS_A800_CORINA_H

#pragma once

#include "rom.h"
//#include "machine/intelfsh.h"

class a800_rom_corina_device : public a800_rom_device
{
public:
	a800_rom_corina_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	a800_rom_corina_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
	virtual void cctl_map(address_map &map) override ATTR_COLD;
	virtual std::tuple<int, int> get_initial_rd_state() override { return std::make_tuple(1, 1); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	std::unique_ptr<uint8_t[]> m_nvram_ptr;

	virtual uint8_t read_view_1(offs_t offset);
	virtual void write_view_1(offs_t offset, u8 data);

	void ctrl_w(offs_t offset, u8 data);

	required_device<nvram_device> m_nvram;
	memory_view m_view;
	u8 m_rom_bank;
};

class a800_rom_corina_sram_device : public a800_rom_corina_device
{
public:
	// construction/destruction
	a800_rom_corina_sram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);


protected:
	virtual void device_start() override ATTR_COLD;

	virtual uint8_t read_view_1(offs_t offset) override;
	virtual void write_view_1(offs_t offset, u8 data) override;

private:
	std::vector<uint8_t> m_ram;
};

DECLARE_DEVICE_TYPE(A800_ROM_CORINA,      a800_rom_corina_device)
DECLARE_DEVICE_TYPE(A800_ROM_CORINA_SRAM, a800_rom_corina_sram_device)

#endif // MAME_BUS_A800_CORINA_H
