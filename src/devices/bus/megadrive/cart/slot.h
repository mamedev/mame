// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_MEGADRIVE_CART_SLOT_H
#define MAME_BUS_MEGADRIVE_CART_SLOT_H

#pragma once

#include "imagedev/cartrom.h"

class device_megadrive_cart_interface;

class megadrive_cart_slot_device : public device_t,
								public device_memory_interface,
								public device_cartrom_image_interface,
								public device_single_card_slot_interface<device_megadrive_cart_interface>
{
public:
	// construction/destruction
	template <typename T>
	megadrive_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock, T &&opts, char const *dflt)
		: megadrive_cart_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	megadrive_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	virtual ~megadrive_cart_slot_device();

	// TODO: should be false for megapc
	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "megadriv_cart"; }
	virtual const char *file_extensions() const noexcept override { return "smd,bin,md,gen"; }

	u16 base_r(offs_t offset, u16 mem_mask);
	void base_w(offs_t offset, u16 data, u16 mem_mask);

	u16 time_r(offs_t offset, u16 mem_mask);
	void time_w(offs_t offset, u16 data, u16 mem_mask);

	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

	void install_cart_mem();

	// from slot to host
	auto vres_cb() { return m_vres_cb.bind(); }

	// from cart to slot
	void vres_w(int state) { m_vres_cb(state); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;
	static const char *get_cart_type(const uint8_t *ROM, uint32_t len);
private:
	device_megadrive_cart_interface *m_cart;

	friend class device_megadrive_cart_interface;

	address_space_config m_space_mem_config;
	address_space_config m_space_io_config;

	address_space *m_space_mem;
	address_space *m_space_io;

	std::error_condition load_swlist();
	std::error_condition load_loose(util::random_read &file);

	devcb_write_line m_vres_cb;
};

class device_megadrive_cart_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_megadrive_cart_interface();

	bool loaded_through_softlist() const { return m_slot && m_slot->loaded_through_softlist(); }
	char const *get_feature(std::string_view feature_name) const { return m_slot ? m_slot->get_feature(feature_name) : nullptr; }

	memory_region *cart_rom_region() { return m_slot ? m_slot->memregion("rom") : nullptr; }
	memory_region *cart_sram_region() { return m_slot ? m_slot->memregion("sram") : nullptr; }

	void battery_load(void *buffer, int length, int fill) { assert(m_slot); m_slot->battery_load(buffer, length, fill); }
	void battery_load(void *buffer, int length, void *def_buffer) { assert(m_slot); m_slot->battery_load(buffer, length, def_buffer); }
	void battery_save(void const *buffer, int length) { assert(m_slot); m_slot->battery_save(buffer, length); }

	virtual std::error_condition load() ATTR_COLD { return std::error_condition(); };
	virtual void unload() ATTR_COLD { };

protected:
	device_megadrive_cart_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override;
	virtual void interface_post_start() override;
	virtual void interface_pre_reset() override;
	megadrive_cart_slot_device *const m_slot;

	virtual void cart_map(address_map &map) ATTR_COLD;
	virtual void time_io_map(address_map &map) ATTR_COLD;

private:
	bool m_cold_reset;
	// device_start, /MRES B2
	// device_reset, /VRES B27
};


DECLARE_DEVICE_TYPE(MEGADRIVE_CART_SLOT, megadrive_cart_slot_device)


#endif // MAME_BUS_MEGADRIVE_CART_SLOT_H
