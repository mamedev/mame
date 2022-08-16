// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_MONONCOL_SLOT_H
#define MAME_BUS_MONONCOL_SLOT_H

#pragma once

#include "imagedev/cartrom.h"

#include <cassert>


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

class device_mononcol_cart_interface : public device_interface
{
public:

	// construction/destruction
	virtual ~device_mononcol_cart_interface();

	// reading and writing
	virtual u8 read_rom(offs_t offset);
	virtual u16 read16_rom(offs_t offset, u16 mem_mask);
	virtual u32 read32_rom(offs_t offset, u32 mem_mask);

	virtual u8 read_ram(offs_t offset);
	virtual void write_ram(offs_t offset, u8 data);

	virtual void rom_alloc(u32 size, int width, endianness_t end, char const *tag);
	virtual void ram_alloc(u32 size);

	u8 *get_rom_base()  { return m_rom; }
	u32 get_rom_size() { return m_rom_size; }

	u8 *get_region_base()  { return m_region.found() ? m_region->base() : nullptr; }
	u32 get_region_size() { return m_region.found() ? m_region->bytes() : 0U; }

	u8 *get_ram_base() { return &m_ram[0]; }
	u32 get_ram_size() { return m_ram.size(); }

	void save_ram() { device().save_item(NAME(m_ram)); }

protected:
	device_mononcol_cart_interface(machine_config const &mconfig, device_t &device);

	// this replaces m_rom for non-user configurable carts!
	optional_memory_region  m_region;

	// internal state
	std::vector<u8> m_ram;
	u8 *m_rom;
	u32 m_rom_size;
};


enum
{
	MONONCOL_ROM8_WIDTH = 1,
	MONONCOL_ROM16_WIDTH = 2,
	MONONCOL_ROM32_WIDTH = 4
};

#define MONONCOL_ROM_REGION_TAG ":cart:rom"



class mononcol_slot_device : public device_t,
								public device_rom_image_interface,
								public device_single_card_slot_interface<device_mononcol_cart_interface>
{
public:
	virtual ~mononcol_slot_device();

	template <typename... T> void set_device_load(T &&... args) { m_device_image_load.set(std::forward<T>(args)...); }
	template <typename... T> void set_device_unload(T &&... args) { m_device_image_unload.set(std::forward<T>(args)...); }

	void set_interface(char const *interface) { m_interface = interface; }
	void set_default_card(char const *def) { m_default_card = def; }
	void set_extensions(char const *exts) { m_extensions = exts; }
	void set_width(int width) { m_width = width; }
	void set_endian(endianness_t end) { m_endianness = end; }

	// device_image_interface implementation
	virtual image_init_result call_load() override;
	virtual void call_unload() override;
	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual char const *image_interface() const noexcept override { return m_interface; }
	virtual char const *file_extensions() const noexcept override { return m_extensions; }

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	u32 common_get_size(char const *region);
	void common_load_rom(u8 *ROM, u32 len, char const *region);

	// reading and writing
	virtual u8 read_rom(offs_t offset);
	virtual u16 read16_rom(offs_t offset, u16 mem_mask = 0xffff);
	virtual u32 read32_rom(offs_t offset, u32 mem_mask = 0xffffffff);

	virtual u8 read_ram(offs_t offset);
	virtual void write_ram(offs_t offset, u8 data);

	virtual void rom_alloc(u32 size, int width, endianness_t end) { if (m_cart) m_cart->rom_alloc(size, width, end, tag()); }
	virtual void ram_alloc(u32 size) { if (m_cart) m_cart->ram_alloc(size); }

	u8* get_rom_base()
	{
		if (!m_cart)
			return nullptr;
		else if (!user_loadable())
			return m_cart->get_region_base();
		else
			return m_cart->get_rom_base();
	}
	u32 get_rom_size()
	{
		if (!m_cart)
			return 0U;
		else if (!user_loadable())
			return m_cart->get_region_size();
		else
			return m_cart->get_rom_size();
	}
	u8 *get_ram_base() { return m_cart ? m_cart->get_ram_base() : nullptr; }

	void save_ram() { if (m_cart && m_cart->get_ram_size()) m_cart->save_ram(); }

protected:
	mononcol_slot_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	char const *m_interface;
	char const *m_default_card;
	char const *m_extensions;
	int m_width;
	endianness_t m_endianness;
	device_mononcol_cart_interface *m_cart;
	load_delegate m_device_image_load;
	unload_delegate m_device_image_unload;
};

class mononcol_socket_device : public mononcol_slot_device
{
public:
	template <typename T>
	mononcol_socket_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *intf, char const *exts = nullptr)
		: mononcol_socket_device(mconfig, tag, owner, u32(0))
	{
		opts(*this);
		set_fixed(false);
		set_interface(intf);
		if (exts)
			set_extensions(exts);
	}

	mononcol_socket_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	virtual const char *image_type_name() const noexcept override { return "romimage"; }
	virtual const char *image_brief_type_name() const noexcept override { return "rom"; }
};

class mononcol_cartslot_device : public mononcol_slot_device
{
public:
	template <typename T>
	mononcol_cartslot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *intf, char const *exts = nullptr)
		: mononcol_cartslot_device(mconfig, tag, owner, u32(0))
	{
		opts(*this);
		set_fixed(false);
		set_interface(intf);
		if (exts)
			set_extensions(exts);
	}

	mononcol_cartslot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	virtual const char *image_type_name() const noexcept override { return "cartridge"; }
	virtual const char *image_brief_type_name() const noexcept override { return "cart"; }
};


// device type definition
DECLARE_DEVICE_TYPE(MONONCOL_SOCKET, mononcol_socket_device)
DECLARE_DEVICE_TYPE(MONONCOL_CARTSLOT, mononcol_cartslot_device)

#endif // MAME_BUS_MONONCOL_SLOT_H
