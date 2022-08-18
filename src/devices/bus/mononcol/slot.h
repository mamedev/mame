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
	virtual void rom_alloc(u32 size, int width, endianness_t end, char const *tag);

	u8 *get_rom_base()  { return m_rom; }
	u32 get_rom_size() { return m_rom_size; }

	u8 *get_region_base()  { return m_region.found() ? m_region->base() : nullptr; }
	u32 get_region_size() { return m_region.found() ? m_region->bytes() : 0U; }

	virtual uint8_t read() { return 0x00; }
	virtual DECLARE_WRITE_LINE_MEMBER(dir_w) { }
	virtual void write(uint8_t data) { };
	virtual void set_ready() { };

	virtual void set_spi_region(uint8_t* region) { }
	virtual void set_spi_size(size_t size) { }

protected:
	device_mononcol_cart_interface(machine_config const &mconfig, device_t &device);

	// this replaces m_rom for non-user configurable carts!
	optional_memory_region  m_region;

	// internal state
	u8 *m_rom;
	u32 m_rom_size;
};


enum
{
	MONONCOL_ROM8_WIDTH = 1,
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

	// device_image_interface implementation
	virtual image_init_result call_load() override;
	virtual void call_unload() override;
	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual char const *image_interface() const noexcept override { return "monon_color_cart"; }
	virtual char const *file_extensions() const noexcept override { return "bin"; }

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	u32 common_get_size(char const *region);
	void common_load_rom(u8 *ROM, u32 len, char const *region);

	virtual void rom_alloc(u32 size, int width, endianness_t end) { if (m_cart) m_cart->rom_alloc(size, width, end, tag()); }

	u8* get_rom_base()
	{
		if (!m_cart)
			return nullptr;

		return m_cart->get_rom_base();
	}
	u32 get_rom_size()
	{
		if (!m_cart)
			return 0U;

		return m_cart->get_rom_size();
	}

	uint8_t read()
	{
		return m_cart->read();
	}

	DECLARE_WRITE_LINE_MEMBER(dir_w)
	{
		m_cart->dir_w(state);
	}

	void set_ready()
	{
		m_cart->set_ready();
	}

	void write(uint8_t data)
	{
		m_cart->write(data);
	}

	void set_spi_region(uint8_t* region) { m_cart->set_spi_region(region); }
	void set_spi_size(size_t size) { m_cart->set_spi_size(size); }


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

class mononcol_cartslot_device : public mononcol_slot_device
{
public:
	template <typename T>
	mononcol_cartslot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts)
		: mononcol_cartslot_device(mconfig, tag, owner, u32(0))
	{
		opts(*this);
		set_fixed(false);
	}

	mononcol_cartslot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	virtual const char *image_type_name() const noexcept override { return "cartridge"; }
	virtual const char *image_brief_type_name() const noexcept override { return "cart"; }
};


// device type definition
DECLARE_DEVICE_TYPE(MONONCOL_CARTSLOT, mononcol_cartslot_device)

#endif // MAME_BUS_MONONCOL_SLOT_H
