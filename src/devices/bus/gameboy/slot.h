// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

 Game Boy/Mega Duck cartridge slot interface

 ***************************************************************************/
#ifndef MAME_BUS_GAMEBOY_SLOT_H
#define MAME_BUS_GAMEBOY_SLOT_H

#include "imagedev/cartrom.h"

#include <cassert>
#include <string>
#include <string_view>
#include <utility>


//**************************************************************************
//  FORWARD DECLARATIONS
//**************************************************************************

class device_gb_cart_interface;



//**************************************************************************
//  CLASS DECLARATIONS
//**************************************************************************

class gb_cart_slot_device_base :
		public device_t,
		public device_cartrom_image_interface,
		public device_single_card_slot_interface<device_gb_cart_interface>
{
public:
	// configuration
	template <typename... T> void set_space(T &&... args) { m_space.set_tag(std::forward<T>(args)...); }

	// device_image_interface implementation
	virtual image_init_result call_load() override ATTR_COLD;
	virtual void call_unload() override ATTR_COLD;
	virtual bool is_reset_on_load() const noexcept override { return true; }

protected:
	gb_cart_slot_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	virtual image_init_result load_image_file(util::random_read &file) = 0;

	address_space &cart_space() noexcept { return *m_space; }

private:
	required_address_space m_space;

	device_gb_cart_interface *m_cart;

	friend class device_gb_cart_interface;
};


class device_gb_cart_interface : public device_interface
{
public:
	virtual image_init_result load(std::string &message) ATTR_COLD = 0;
	virtual void unload() ATTR_COLD;

protected:
	device_gb_cart_interface(machine_config const &mconfig, device_t &device);

	bool has_slot() const noexcept { return bool(m_slot); }
	address_space *cart_space() noexcept { return m_slot ? m_slot->m_space.target() : nullptr; }

	bool loaded_through_softlist() const { return m_slot && m_slot->loaded_through_softlist(); }
	char const *get_feature(std::string_view feature_name) const { return m_slot ? m_slot->get_feature(feature_name) : nullptr; }

	memory_region *cart_rom_region() { return m_slot ? m_slot->memregion("rom") : nullptr; }
	memory_region *cart_ram_region() { return m_slot ? m_slot->memregion("ram") : nullptr; }
	memory_region *cart_nvram_region() { return m_slot ? m_slot->memregion("nvram") : nullptr; }
	memory_region *gbx_footer_region() { return m_slot ? m_slot->memregion("gbx") : nullptr; }

	void battery_load(void *buffer, int length, int fill) { assert(m_slot); m_slot->battery_load(buffer, length, fill); }
	void battery_load(void *buffer, int length, void *def_buffer) { assert(m_slot); m_slot->battery_load(buffer, length, def_buffer); }
	void battery_save(void const *buffer, int length) { assert(m_slot); m_slot->battery_save(buffer, length); }

private:
	gb_cart_slot_device_base *const m_slot;
};

#endif // MAME_BUS_GAMEBOY_SLOT_H
