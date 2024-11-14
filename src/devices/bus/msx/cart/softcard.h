// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_SOFTCARD_H
#define MAME_BUS_MSX_CART_SOFTCARD_H

#pragma once

#include "bus/msx/slot/cartridge.h"
#include "imagedev/cartrom.h"


DECLARE_DEVICE_TYPE(MSX_CART_SOFTCARD, msx_cart_softcard_device)

class softcard_interface;


class msx_cart_softcard_device : public device_t
								, public device_cartrom_image_interface
								, public device_single_card_slot_interface<softcard_interface>
								, public msx_cart_interface
{
public:
	msx_cart_softcard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "softcard"; }
	virtual const char *file_extensions() const noexcept override { return "bin,rom"; }

protected:
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override { }
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	softcard_interface *m_softcard;
};


class softcard_interface : public device_interface
{
public:
	virtual std::error_condition initialize_cartridge(std::string &message) { return std::error_condition(); }
	void set_views(memory_view::memory_view_entry *page0, memory_view::memory_view_entry *page1, memory_view::memory_view_entry *page2, memory_view::memory_view_entry *page3);

protected:
	softcard_interface(const machine_config &mconfig, device_t &device);
	memory_region *cart_rom_region() { return m_slot ? m_slot->memregion("rom") : nullptr; }
	memory_view::memory_view_entry *page(int i) { return m_page[i]; }

private:
	memory_view::memory_view_entry *m_page[4];
	msx_cart_softcard_device *const m_slot;
};


#endif // MAME_BUS_MSX_CART_SOFTCARD_H
