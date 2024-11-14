// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_BUS_JAKKS_GAMEKEY_SLOT_H
#define MAME_BUS_JAKKS_GAMEKEY_SLOT_H

#pragma once

#include "imagedev/cartrom.h"

/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

/* PCB */
enum
{
	JAKKS_GAMEKEY_PLAIN = 0,
	JAKKS_GAMEKEY_I2C_BASE,
	JAKKS_GAMEKEY_I2C_24LC04,
};

// ======================> device_jakks_gamekey_interface

class device_jakks_gamekey_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_jakks_gamekey_interface();

	// reading and writing
	virtual uint16_t read_cart(offs_t offset) { return 0xffff; }
	virtual void write_cart(offs_t offset, uint16_t data) { }

	virtual uint8_t read_cart_seeprom(void) { return 1; }
	virtual void write_cart_seeprom(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { }

	void rom_alloc(uint32_t size, const char *tag);
	uint8_t* get_rom_base() { return m_rom; }
	uint32_t get_rom_size() { return m_rom_size; }

protected:
	device_jakks_gamekey_interface(const machine_config &mconfig, device_t &device);

	// internal state
	uint8_t *m_rom;
	uint32_t m_rom_size;
};

// ======================> jakks_gamekey_slot_device

class jakks_gamekey_slot_device : public device_t,
								public device_cartrom_image_interface,
								public device_single_card_slot_interface<device_jakks_gamekey_interface>
{
public:
	// construction/destruction
	jakks_gamekey_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T>
	jakks_gamekey_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&opts, const char *dflt)
		: jakks_gamekey_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	virtual ~jakks_gamekey_slot_device();

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override { }

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "jakks_gamekey"; }
	virtual const char *file_extensions() const noexcept override { return "bin,u1"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	int get_type() { return m_type; }
	static int get_cart_type(const uint8_t *ROM, uint32_t len);

	// reading and writing
	uint16_t read_cart(offs_t offset);
	void write_cart(offs_t offset, uint16_t data);

	virtual uint8_t read_cart_seeprom(void);
	virtual void write_cart_seeprom(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	bool has_cart() { return m_cart ? true : false; }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	int m_type;
	device_jakks_gamekey_interface *m_cart;
};

// device type definition
DECLARE_DEVICE_TYPE(JAKKS_GAMEKEY_SLOT, jakks_gamekey_slot_device)

/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

#define JAKKSSLOT_ROM_REGION_TAG ":cart:rom"

void jakks_gamekey(device_slot_interface &device);

#endif // MAME_BUS_JAKKS_GAMEKEY_SLOT_H
