// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_MEGADRIVE_CART_SRAM_H
#define MAME_BUS_MEGADRIVE_CART_SRAM_H

#pragma once

#include "machine/nvram.h"

#include "rom.h"
#include "slot.h"

class megadrive_rom_sram_device : public megadrive_rom_device
{
public:
	megadrive_rom_sram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
	virtual void time_io_map(address_map &map) override ATTR_COLD;

protected:
	megadrive_rom_sram_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual std::error_condition load() override ATTR_COLD;
	virtual void unload() override ATTR_COLD;

	memory_view m_sram_view;
private:
	u8 *m_nvram_base;
	u32 m_nvram_size, m_nvram_mask;

	bool m_nvram_write_protect;
};

class megadrive_rom_sonic3_device : public megadrive_rom_sram_device
{
public:
	megadrive_rom_sonic3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void time_io_map(address_map &map) override ATTR_COLD;
};

class megadrive_rom_tplay96_device : public megadrive_rom_device
{
public:
	megadrive_rom_tplay96_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;

protected:
	megadrive_rom_tplay96_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual u16 get_nvram_length() ATTR_COLD;

	u16 nvram_r(offs_t offset);
	void nvram_w(offs_t offset, u16 data, u16 mem_mask);

private:
	required_device<nvram_device> m_nvram;
	std::unique_ptr<uint8_t[]> m_nvram_ptr;
	u16 m_nvram_mask;
};

class megadrive_rom_hardball95_device : public megadrive_rom_tplay96_device
{
public:
	megadrive_rom_hardball95_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
};

class megadrive_rom_barkley2_device : public megadrive_rom_tplay96_device
{
public:
	megadrive_rom_barkley2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
protected:
	virtual u16 get_nvram_length() override ATTR_COLD;
};

class megadrive_unl_sanguo5_device : public megadrive_rom_device
{
public:
	megadrive_unl_sanguo5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	u16 nvram_r(offs_t offset);
	void nvram_w(offs_t offset, u16 data, u16 mem_mask);

private:
	required_device<nvram_device> m_nvram;
	std::unique_ptr<uint8_t[]> m_nvram_ptr;
};


DECLARE_DEVICE_TYPE(MEGADRIVE_ROM_SRAM,       megadrive_rom_sram_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_ROM_SONIC3,     megadrive_rom_sonic3_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_ROM_TPLAY96,    megadrive_rom_tplay96_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_ROM_HARDBALL95, megadrive_rom_hardball95_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_ROM_BARKLEY2,   megadrive_rom_barkley2_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_UNL_SANGUO5,    megadrive_unl_sanguo5_device)

#endif // MAME_BUS_MEGADRIVE_CART_SRAM_H
