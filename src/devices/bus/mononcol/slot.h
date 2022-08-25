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
	virtual void rom_alloc(u32 size, int width, endianness_t end);

	u8 *get_rom_base()  { return m_rom; }
	u32 get_rom_size() { return m_rom_size; }

	virtual uint8_t read() = 0;
	virtual DECLARE_WRITE_LINE_MEMBER(dir_w) = 0;
	virtual void write(uint8_t data) = 0;
	virtual void set_ready() = 0;

	virtual void set_spi_region(uint8_t* region) = 0;
	virtual void set_spi_size(size_t size) = 0;

protected:
	device_mononcol_cart_interface(machine_config const &mconfig, device_t &device);

	// internal state
	u8 *m_rom;
	u32 m_rom_size;
};


class mononcol_cartslot_device : public device_t,
								public device_rom_image_interface,
								public device_single_card_slot_interface<device_mononcol_cart_interface>
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

	virtual ~mononcol_cartslot_device();

	// device_image_interface implementation
	virtual image_init_result call_load() override;
	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual char const *image_interface() const noexcept override { return "monon_color_cart"; }
	virtual char const *file_extensions() const noexcept override { return "bin"; }

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	u32 common_get_size(char const *region);
	void common_load_rom(u8 *ROM, u32 len, char const *region);

	virtual void rom_alloc(u32 size, int width, endianness_t end) { if (m_cart) m_cart->rom_alloc(size, width, end); }

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
	mononcol_cartslot_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	device_mononcol_cart_interface *m_cart;

	virtual const char *image_type_name() const noexcept override { return "cartridge"; }
	virtual const char *image_brief_type_name() const noexcept override { return "cart"; }

};


// device type definition
DECLARE_DEVICE_TYPE(MONONCOL_CARTSLOT, mononcol_cartslot_device)

#endif // MAME_BUS_MONONCOL_SLOT_H
