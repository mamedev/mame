// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_BUS_GAMATE_SLOT_H
#define MAME_BUS_GAMATE_SLOT_H

#pragma once

#include "imagedev/cartrom.h"

/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

/* PCB */
enum
{
	GAMATE_PLAIN = 0,
	GAMATE_BANKED,
	GAMATE_4IN1,
};

// ======================> device_gamate_cart_interface

class device_gamate_cart_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_gamate_cart_interface();

	// reading and writing
	virtual uint8_t read_cart(offs_t offset) { return 0xff; }
	virtual void write_cart(offs_t offset, uint8_t data) { }

	void rom_alloc(uint32_t size);
	uint8_t* get_rom_base() { return m_rom; }
	uint32_t get_rom_size() { return m_rom_size; }

protected:
	device_gamate_cart_interface(const machine_config &mconfig, device_t &device);

	// internal state
	uint8_t *m_rom;
	uint32_t m_rom_size;
};

// ======================> gamate_cart_slot_device

class gamate_cart_slot_device : public device_t,
								public device_cartrom_image_interface,
								public device_single_card_slot_interface<device_gamate_cart_interface>
{
public:
	// construction/destruction
	template <typename T>
	gamate_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: gamate_cart_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	gamate_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~gamate_cart_slot_device();

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override { }

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "gamate_cart"; }
	virtual const char *file_extensions() const noexcept override { return "bin"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	int get_type() { return m_type; }
	static int get_cart_type(const uint8_t *ROM, uint32_t len);

	// reading and writing
	uint8_t read_cart(offs_t offset);
	void write_cart(offs_t offset, uint8_t data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	int m_type;
	device_gamate_cart_interface *m_cart;
};

// device type definition
DECLARE_DEVICE_TYPE(GAMATE_CART_SLOT, gamate_cart_slot_device)

/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

void gamate_cart(device_slot_interface &device);

#endif // MAME_BUS_GAMATE_SLOT_H
