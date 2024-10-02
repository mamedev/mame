// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_SEGAAI_SLOT_H
#define MAME_BUS_SEGAAI_SLOT_H

#pragma once

#include "imagedev/cartrom.h"


DECLARE_DEVICE_TYPE(SEGAAI_CARD_SLOT, segaai_card_slot_device);


class segaai_card_interface;


class segaai_card_slot_device : public device_t,
								public device_cartrom_image_interface,
								public device_single_card_slot_interface<segaai_card_interface>
{
public:
	template <typename T>
	segaai_card_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: segaai_card_slot_device(mconfig, tag, owner, u32(0))
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	segaai_card_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	virtual ~segaai_card_slot_device();

	template <typename T> void set_address_space(T &&tag, int no) { m_address_space.set_tag(std::forward<T>(tag), no); }
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override {}
	virtual const char *image_type_name() const noexcept override { return "card"; }
	virtual const char *image_brief_type_name() const noexcept override { return "card"; }

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "segaai_card"; }
	virtual const char *file_extensions() const noexcept override { return "aic,bin"; }

	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

protected:
	virtual void device_start() override ATTR_COLD;

private:
	optional_address_space m_address_space;
	segaai_card_interface* m_cart;
};


class segaai_card_interface : public device_interface
{
public:
	segaai_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~segaai_card_interface();

	virtual void install_memory_handlers(address_space *space) { }

protected:
	memory_region *cart_rom_region() { return m_slot ? m_slot->memregion("rom") : nullptr; }

private:
	segaai_card_slot_device *const m_slot;
};


void segaai_cards(device_slot_interface &device);

#endif
