// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn
#ifndef MAME_BUS_XAVIX_CART_H
#define MAME_BUS_XAVIX_CART_H

#pragma once

#include "imagedev/cartrom.h"

#include <string>
#include <system_error>
#include <utility>

class device_xavix_cart_interface : public device_interface
{
public:
	virtual ~device_xavix_cart_interface();

	virtual uint8_t read_cart(offs_t offset);
	virtual void write_cart(offs_t offset, uint8_t data);

	virtual void write_sda(int state);
	virtual void write_scl(int state);
	virtual int read_sda();

	virtual void write_bus_control(offs_t offset, uint8_t data);

	void rom_alloc(uint32_t size);
	uint8_t *get_rom_base() { return m_rom; }

protected:
	device_xavix_cart_interface(const machine_config &mconfig, device_t &device);

	uint8_t *m_rom;
	uint32_t m_rom_size;
};

class xavix_cart_slot_device : public device_t,
								public device_cartrom_image_interface,
								public device_single_card_slot_interface<device_xavix_cart_interface>
{
public:
	xavix_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <typename T>
	xavix_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: xavix_cart_slot_device(mconfig, tag, owner)
	{
		set_options(std::forward<T>(opts), dflt, false);
	}

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual bool is_reset_on_load() const noexcept override;
	virtual const char *image_interface() const noexcept override;
	virtual const char *file_extensions() const noexcept override;

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	uint8_t read_cart(offs_t offset);
	void write_cart(offs_t offset, uint8_t data);

	void write_sda(int state);
	void write_scl(int state);
	int read_sda();

	void write_bus_control(offs_t offset, uint8_t data);

	bool has_cart() const { return m_cart != nullptr; }

	static void cart_devices(device_slot_interface &device) ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	device_xavix_cart_interface *m_cart;
};

DECLARE_DEVICE_TYPE(XAVIX_CART_SLOT, xavix_cart_slot_device)

#endif // MAME_BUS_XAVIX_CART_H
