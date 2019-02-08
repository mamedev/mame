// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_BUS_EKARA_SLOT_H
#define MAME_BUS_EKARA_SLOT_H

#pragma once

#include "softlist_dev.h"

/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

/* PCB */
enum
{
	EKARA_PLAIN = 0,
	EKARA_I2C_BASE,
	EKARA_I2C_24C08,
	EKARA_I2C_24LC04,
	EKARA_I2C_24LC02,
};

// ======================> device_ekara_cart_interface

class device_ekara_cart_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	virtual ~device_ekara_cart_interface();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_cart) { return 0xff; }
	virtual DECLARE_WRITE8_MEMBER(write_cart) { }

	virtual DECLARE_READ8_MEMBER(read_extra) { return 0xff; }
	virtual DECLARE_WRITE8_MEMBER(write_extra) { }

	virtual DECLARE_WRITE8_MEMBER(write_bus_control) { }

	virtual bool is_read_access_not_rom(void) { return false; }
	virtual bool is_write_access_not_rom(void) { return false; }

	void rom_alloc(uint32_t size, const char *tag);
	uint8_t* get_rom_base() { return m_rom; }
	uint32_t get_rom_size() { return m_rom_size; }

protected:
	device_ekara_cart_interface(const machine_config &mconfig, device_t &device);

	// internal state
	uint8_t *m_rom;
	uint32_t m_rom_size;
};

// ======================> ekara_cart_slot_device

class ekara_cart_slot_device : public device_t,
								public device_image_interface,
								public device_slot_interface
{
public:
	// construction/destruction
	ekara_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T>
	ekara_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&opts, const char *dflt)
		: ekara_cart_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	virtual ~ekara_cart_slot_device();

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual void call_unload() override {}
	virtual const software_list_loader &get_software_list_loader() const override { return rom_software_list_loader::instance(); }

	int get_type() { return m_type; }
	static int get_cart_type(const uint8_t *ROM, uint32_t len);

	virtual iodevice_t image_type() const override { return IO_CARTSLOT; }
	virtual bool is_readable()  const override { return 1; }
	virtual bool is_writeable() const override { return 0; }
	virtual bool is_creatable() const override { return 0; }
	virtual bool must_be_loaded() const override { return 0; }
	virtual bool is_reset_on_load() const override { return 1; }
	virtual const char *image_interface() const override { return "ekara_cart"; }
	virtual const char *file_extensions() const override { return "bin,u1"; }

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_cart);
	virtual DECLARE_WRITE8_MEMBER(write_cart);

	virtual DECLARE_READ8_MEMBER(read_extra);
	virtual DECLARE_WRITE8_MEMBER(write_extra);

	virtual DECLARE_WRITE8_MEMBER(write_bus_control);

	virtual bool is_read_access_not_rom(void);
	virtual bool is_write_access_not_rom(void);

	bool has_cart() { return m_cart ? true : false; }

protected:
	// device-level overrides
	virtual void device_start() override;

	int m_type;
	device_ekara_cart_interface*       m_cart;
};

// device type definition
DECLARE_DEVICE_TYPE(EKARA_CART_SLOT, ekara_cart_slot_device)

/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

#define EKARASLOT_ROM_REGION_TAG ":cart:rom"

void ekara_cart(device_slot_interface &device);

#endif // MAME_BUS_EKARA_SLOT_H
