// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
#ifndef MAME_BUS_XAVIX_SUPRTVPC_CART_H
#define MAME_BUS_XAVIX_SUPRTVPC_CART_H

#pragma once

#include "imagedev/cartrom.h"
#include "machine/i2cmem.h"

#include <string>
#include <system_error>
#include <utility>

class device_suprtvpc_cart_interface : public device_interface
{
public:
	virtual ~device_suprtvpc_cart_interface();

	virtual void sda_w(int state) { }
	virtual void scl_w(int state) { }
	virtual int sda_r() { return 1; }
	virtual void ir_tx_w(int state) { }

	void rom_alloc(u32 size);
	u8 *rom_base() const { return m_rom; }
	u32 rom_size() const { return m_rom_size; }

protected:
	device_suprtvpc_cart_interface(const machine_config &mconfig, device_t &device);
	void ir_rx_pulse();

private:
	u8 *m_rom;
	u32 m_rom_size;
};

class suprtvpc_cart_slot_device : public device_t,
		public device_cartrom_image_interface,
		public device_single_card_slot_interface<device_suprtvpc_cart_interface>
{
public:
	template <typename T>
	suprtvpc_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: suprtvpc_cart_slot_device(mconfig, tag, owner, 0)
	{
		set_options(std::forward<T>(opts), dflt, false);
	}

	suprtvpc_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	u8 *rom_base() const { return m_cart ? m_cart->rom_base() : nullptr; }
	u32 rom_size() const { return m_cart ? m_cart->rom_size() : 0; }
	memory_region *rom_region() const { return memregion("cart:rom"); }

	void sda_w(int state) { if (m_cart) m_cart->sda_w(state); }
	void scl_w(int state) { if (m_cart) m_cart->scl_w(state); }
	int sda_r() { return m_cart ? m_cart->sda_r() : 1; }
	void ir_tx_w(int state) { if (m_cart) m_cart->ir_tx_w(state); }
	template <typename... T> void set_ir_rx_callback(T &&... args) { m_ir_rx_cb.set(std::forward<T>(args)...); }
	void ir_rx_pulse() { m_ir_rx_cb(); }

	static void cart_devices(device_slot_interface &device) ATTR_COLD;

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual bool is_reset_on_load() const noexcept override;
	virtual const char *image_interface() const noexcept override;
	virtual const char *file_extensions() const noexcept override;

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	device_suprtvpc_cart_interface *m_cart;
	device_delegate<void ()> m_ir_rx_cb;
};

class suprtvpc_rom_device : public device_t, public device_suprtvpc_cart_interface
{
public:
	suprtvpc_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	suprtvpc_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD { }
};

class suprtvpc_rom_i2c_device : public suprtvpc_rom_device
{
public:
	// device_suprtvpc_cart_interface implementation
	virtual void sda_w(int state) override;
	virtual void scl_w(int state) override;
	virtual int sda_r() override;

protected:
	suprtvpc_rom_i2c_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	required_device<i2cmem_device> m_eeprom;
};

class suprtvpc_rom_24c04_device : public suprtvpc_rom_i2c_device
{
public:
	suprtvpc_rom_24c04_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class suprtvpc_rom_24c08_device : public suprtvpc_rom_i2c_device
{
public:
	suprtvpc_rom_24c08_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class suprtvpc_rom_24c64_device : public suprtvpc_rom_i2c_device
{
public:
	suprtvpc_rom_24c64_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(SUPRTVPC_CART_SLOT, suprtvpc_cart_slot_device)
DECLARE_DEVICE_TYPE(SUPRTVPC_ROM, suprtvpc_rom_device)
DECLARE_DEVICE_TYPE(SUPRTVPC_ROM_24C04, suprtvpc_rom_24c04_device)
DECLARE_DEVICE_TYPE(SUPRTVPC_ROM_24C08, suprtvpc_rom_24c08_device)
DECLARE_DEVICE_TYPE(SUPRTVPC_ROM_24C64, suprtvpc_rom_24c64_device)

#endif // MAME_BUS_XAVIX_SUPRTVPC_CART_H
