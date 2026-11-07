// license:BSD-3-Clause
// copyright-holders: Peter Wilhelmsen
#ifndef MAME_BUS_DIGIBLAST_SLOT_H
#define MAME_BUS_DIGIBLAST_SLOT_H

#pragma once

#include "imagedev/cartrom.h"

#include <utility>

class device_digiblast_cart_interface : public device_interface
{
public:
	virtual u8 data_r() = 0;
	virtual void data_w(u8 data) = 0;
	virtual void command_w(u8 data) = 0;
	virtual void address_w(u8 data) = 0;
	virtual int i2c_sda_r() = 0;
	virtual void i2c_sda_w(int state) = 0;
	virtual void i2c_scl_w(int state) = 0;
	virtual u32 rom_size() const = 0;
	virtual void set_rom(const u8 *data, u32 size) = 0;
	virtual u32 eeprom_size() const = 0;
	virtual void set_default_eeprom(const u8 *data) = 0;

protected:
	device_digiblast_cart_interface(const machine_config &mconfig, device_t &device);
};

class digiblast_cart_slot_device : public device_t,
	public device_cartrom_image_interface,
	public device_single_card_slot_interface<device_digiblast_cart_interface>
{
public:
	template <typename T>
	digiblast_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts)
		: digiblast_cart_slot_device(mconfig, tag, owner, 0U)
	{
		set_options(std::forward<T>(opts), "nand_64m", false);
	}

	digiblast_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto ready_callback() { return m_ready_cb.bind(); }
	void ready_w(int state) { m_ready_cb(state); }

	u8 data_r();
	void data_w(u8 data);
	void command_w(u8 data);
	void address_w(u8 data);
	int i2c_sda_r();
	void i2c_sda_w(int state);
	void i2c_scl_w(int state);

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;
	virtual bool is_reset_on_load() const noexcept override;
	virtual const char *image_interface() const noexcept override;
	virtual const char *file_extensions() const noexcept override;

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	device_digiblast_cart_interface *m_cart;
	devcb_write_line m_ready_cb;
};

void digiblast_cart(device_slot_interface &device);

DECLARE_DEVICE_TYPE(DIGIBLAST_CART_SLOT, digiblast_cart_slot_device)

#endif // MAME_BUS_DIGIBLAST_SLOT_H
