// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_SLOT_CARTRIDGE_H
#define MAME_BUS_MSX_SLOT_CARTRIDGE_H

#pragma once

#include "slot.h"
#include "bus/msx/cart/cartridge.h"
#include "imagedev/cartrom.h"


DECLARE_DEVICE_TYPE(MSX_SLOT_CARTRIDGE,        msx_slot_cartridge_device)
DECLARE_DEVICE_TYPE(MSX_SLOT_YAMAHA_EXPANSION, msx_slot_yamaha_expansion_device)


class msx_cart_interface;

class msx_slot_cartridge_device : public device_t
								, public device_cartrom_image_interface
								, public device_slot_interface
								, public msx_internal_slot_interface
{
public:
	// construction/destruction
	msx_slot_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	auto irq_handler() { return m_irq_handler.bind(); }

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual void call_unload() override;
	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "msx_cart"; }
	virtual const char *file_extensions() const noexcept override { return "mx1,bin,rom"; }

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	DECLARE_WRITE_LINE_MEMBER(irq_out);

protected:
	msx_slot_cartridge_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;

	devcb_write_line m_irq_handler;
	msx_cart_interface *m_cartridge;

	static int get_cart_type(const u8 *rom, u32 length);
};



class msx_slot_yamaha_expansion_device : public msx_slot_cartridge_device
{
public:
	// construction/destruction
	msx_slot_yamaha_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual const char *image_interface() const noexcept override { return "msx_yamaha_60pin"; }
	virtual const char *image_type_name() const noexcept override { return "cartridge60pin"; }
	virtual const char *image_brief_type_name() const noexcept override { return "cart60p"; }

protected:
	virtual void device_start() override;
};



class msx_cart_interface : public device_interface
{
	friend class msx_slot_cartridge_device;

public:
	// This is called after loading cartridge contents and allows the cartridge
	// implementation to perform some additional initialization based on the
	// cartridge contents.
	virtual image_init_result initialize_cartridge(std::string &message) { return image_init_result::PASS; }
	virtual void interface_pre_start() override { assert(m_exp != nullptr); }

	void set_views(memory_view::memory_view_entry *page0, memory_view::memory_view_entry *page1, memory_view::memory_view_entry *page2, memory_view::memory_view_entry *page3);

protected:
	msx_cart_interface(const machine_config &mconfig, device_t &device);

	memory_region *cart_rom_region() { return m_exp ? m_exp->memregion("rom") : nullptr; }
	memory_region *cart_vlm5030_region() { return m_exp ? m_exp->memregion("vlm5030") : nullptr; }
	memory_region *cart_kanji_region() { return m_exp ? m_exp->memregion("kanji") : nullptr; }
	memory_region *cart_ram_region() { return m_exp ? m_exp->memregion("ram") : nullptr; }
	memory_region *cart_sram_region() { return m_exp ? m_exp->memregion("sram") : nullptr; }
	DECLARE_WRITE_LINE_MEMBER(irq_out);
	address_space &memory_space() const;
	address_space &io_space() const;
	cpu_device &maincpu() const;
	memory_view::memory_view_entry *page(int i) { return m_page[i]; }

private:
	msx_slot_cartridge_device *m_exp;
	memory_view::memory_view_entry *m_page[4];
};

#endif // MAME_BUS_MSX_SLOT_CARTRIDGE_H
