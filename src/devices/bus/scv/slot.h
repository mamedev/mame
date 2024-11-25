// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_SCV_SLOT_H
#define MAME_BUS_SCV_SLOT_H

#pragma once

#include "imagedev/cartrom.h"


class device_scv_cart_interface;


// ======================> scv_cart_slot_device

class scv_cart_slot_device : public device_t,
								public device_cartrom_image_interface,
								public device_single_card_slot_interface<device_scv_cart_interface>
{
public:
	// construction/destruction
	template <typename T>
	scv_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: scv_cart_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	scv_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	virtual ~scv_cart_slot_device();

	template <typename T> void set_address_space(T &&tag, int no) { m_address_space.set_tag(std::forward<T>(tag), no); }

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override { }

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "scv_cart"; }
	virtual const char *file_extensions() const noexcept override { return "bin"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	int get_type() { return m_type; }
	static int get_cart_type(u32 len);

	// reading and writing
	void write_bank(u8 data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	int m_type;
	device_scv_cart_interface *m_cart;
	optional_address_space m_address_space;
};


// ======================> device_scv_cart_interface

class device_scv_cart_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_scv_cart_interface();

	virtual void install_memory_handlers(address_space *space) { }
	virtual void write_bank(u8 data) { }

	void savestate_ram();

protected:
	device_scv_cart_interface(const machine_config &mconfig, device_t &device);

	memory_region *cart_rom_region() { return m_slot ? m_slot->memregion("rom") : nullptr; }
	memory_region *cart_ram_region() { return m_slot ? m_slot->memregion("ram") : nullptr; }

private:
	scv_cart_slot_device *const m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(SCV_CART_SLOT, scv_cart_slot_device)

#endif // MAME_BUS_SCV_SLOT_H
