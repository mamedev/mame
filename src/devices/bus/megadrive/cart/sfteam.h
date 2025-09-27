
// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_MEGADRIVE_CART_SFTEAM_H
#define MAME_BUS_MEGADRIVE_CART_SFTEAM_H

#pragma once

#include "machine/nvram.h"

#include "rom.h"
#include "sram.h"
#include "slot.h"


class megadrive_unl_xinqig_device : public megadrive_rom_tplay96_device
{
public:
	megadrive_unl_xinqig_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
protected:
	megadrive_unl_xinqig_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual u16 get_nvram_length() override ATTR_COLD;
};

class megadrive_hb_beggarp_device : public megadrive_unl_xinqig_device
{
public:
	megadrive_hb_beggarp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;

protected:
	megadrive_hb_beggarp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	memory_bank_creator m_sf_rom_bank;
	memory_view m_sf_rom_view;
	memory_view m_sram_view;

	void bank_write_w(offs_t offset, u16 data, u16 mem_mask);
private:
	bool m_bank_write_lock;
};

class megadrive_hb_beggarp1_device : public megadrive_hb_beggarp_device
{
public:
	megadrive_hb_beggarp1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
};

class megadrive_hb_wukong_device : public megadrive_rom_tplay96_device
{
public:
	megadrive_hb_wukong_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual u16 get_nvram_length() override ATTR_COLD;
private:
	memory_bank_creator m_sf_rom_bank;
	memory_view m_sram_view;
};

class megadrive_hb_starodys_device : public megadrive_rom_tplay96_device
{
public:
	megadrive_hb_starodys_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
	virtual void time_io_map(address_map &map) override ATTR_COLD;
protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual u16 get_nvram_length() override ATTR_COLD;
private:
	memory_bank_array_creator<5> m_sf_rom_bank;
	memory_view m_sf_rom_view;
	memory_view m_sram_view;

	u8 m_bank_num;
	bool m_bank_write_lock;
};


DECLARE_DEVICE_TYPE(MEGADRIVE_UNL_XINQIG,     megadrive_unl_xinqig_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_HB_BEGGARP,     megadrive_hb_beggarp_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_HB_BEGGARP1,    megadrive_hb_beggarp1_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_HB_WUKONG,      megadrive_hb_wukong_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_HB_STARODYS,    megadrive_hb_starodys_device)

#endif // MAME_BUS_MEGADRIVE_CART_SFTEAM_H

