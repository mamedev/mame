// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_BUS_JAKKS_GAMEKEY_SLOT_H
#define MAME_BUS_JAKKS_GAMEKEY_SLOT_H

#pragma once

#include "softlist_dev.h"

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
	virtual DECLARE_READ16_MEMBER(read_cart) { return 0xffff; }
	virtual DECLARE_WRITE16_MEMBER(write_cart) { }

	virtual uint8_t read_cart_seeprom(void) { return 1; }
	virtual DECLARE_WRITE16_MEMBER(write_cart_seeprom) { }

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
								public device_image_interface,
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

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual void call_unload() override { }

	virtual iodevice_t image_type() const noexcept override { return IO_CARTSLOT; }
	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return false; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool must_be_loaded() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "jakks_gamekey"; }
	virtual const char *file_extensions() const noexcept override { return "bin,u1"; }

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	int get_type() { return m_type; }
	static int get_cart_type(const uint8_t *ROM, uint32_t len);

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read_cart);
	virtual DECLARE_WRITE16_MEMBER(write_cart);

	virtual uint8_t read_cart_seeprom(void);
	virtual DECLARE_WRITE16_MEMBER(write_cart_seeprom);

	bool has_cart() { return m_cart ? true : false; }

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_image_interface implementation
	virtual const software_list_loader &get_software_list_loader() const override { return rom_software_list_loader::instance(); }

	int m_type;
	device_jakks_gamekey_interface*       m_cart;
};

// device type definition
DECLARE_DEVICE_TYPE(JAKKS_GAMEKEY_SLOT, jakks_gamekey_slot_device)

/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

#define JAKKSSLOT_ROM_REGION_TAG ":cart:rom"

void jakks_gamekey(device_slot_interface &device);

#endif // MAME_BUS_JAKKS_GAMEKEY_SLOT_H
