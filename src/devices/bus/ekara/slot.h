// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_BUS_EKARA_SLOT_H
#define MAME_BUS_EKARA_SLOT_H

#pragma once

#include "imagedev/cartrom.h"

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

class device_ekara_cart_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_ekara_cart_interface();

	// reading and writing
	virtual uint8_t read_cart(offs_t offset) { return 0xff; }
	virtual void write_cart(offs_t offset, uint8_t data) { }

	virtual uint8_t read_extra(offs_t offset) { return 0xff; }
	virtual void write_extra(offs_t offset, uint8_t data) { }

	virtual void write_sda(int state) { }
	virtual void write_scl(int state) { }
	//virtual void write_wc(int state)
	virtual int read_sda() { return 0; }

	virtual void write_bus_control(offs_t offset, uint8_t data) { }

	virtual bool is_read_access_not_rom(void) { return false; }
	virtual bool is_write_access_not_rom(void) { return false; }

	void rom_alloc(uint32_t size);
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
								public device_cartrom_image_interface,
								public device_single_card_slot_interface<device_ekara_cart_interface>
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

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override {}

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "ekara_cart"; }
	virtual const char *file_extensions() const noexcept override { return "bin,u1"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	int get_type() { return m_type; }
	static int get_cart_type(const uint8_t *ROM, uint32_t len);

	// reading and writing
	uint8_t read_cart(offs_t offset);
	void write_cart(offs_t offset, uint8_t data);

	uint8_t read_extra(offs_t offset);
	void write_extra(offs_t offset, uint8_t data);

	void write_sda(int state);
	void write_scl(int state);
	//void write_wc(int state);
	int read_sda();

	void write_bus_control(offs_t offset, uint8_t data);

	bool is_read_access_not_rom(void);
	bool is_write_access_not_rom(void);

	bool has_cart() { return m_cart ? true : false; }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	int m_type;
	device_ekara_cart_interface*       m_cart;
};

// device type definition
DECLARE_DEVICE_TYPE(EKARA_CART_SLOT, ekara_cart_slot_device)

/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

void ekara_cart(device_slot_interface &device);

#endif // MAME_BUS_EKARA_SLOT_H
