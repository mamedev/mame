// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

 MSX cartridge slot

 Cartridge edge connector (double-sided):
    /CS2 -  2  1 - /CS1
  /SLTSL -  4  3 - /CS12
   /RFSH -  6  5 - reserved
    /INT -  8  7 - /WAIT
 /BUSDIR - 10  9 - /M1
   /MREQ - 12 11 - /IORQ
     /RD - 14 13 - /WR
reserved - 16 15 - /RESET
     A15 - 18 17 - A9
     A10 - 20 19 - A11
      A6 - 22 21 - A7
      A8 - 24 23 - A12
     A13 - 26 25 - A14
      A0 - 28 27 - A1
      A2 - 30 29 - A3
      A4 - 32 31 - A5
      D0 - 34 33 - D1
      D2 - 36 35 - D3
      D4 - 38 37 - D5
      D6 - 40 39 - D7
   CLOCK - 42 41 - GND
     SW1 - 44 43 - GND
     SW2 - 46 45 - +5V
    +12V - 48 47 - +5V
    -12V - 50 49 - SOUNDIN

 ***************************************************************************/
#ifndef MAME_BUS_MSX_SLOT_CARTRIDGE_H
#define MAME_BUS_MSX_SLOT_CARTRIDGE_H

#pragma once

#include "slot.h"
#include "imagedev/cartrom.h"


class msx_cart_interface;

class msx_slot_cartridge_base_device : public device_t
								, public device_cartrom_image_interface
								, public device_slot_interface
								, public device_mixer_interface
								, public msx_internal_slot_interface
{
public:
	auto irq_handler() { return m_irq_handler.bind(); }

	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;
	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "msx_cart"; }
	virtual const char *file_extensions() const noexcept override { return "mx1,bin,rom"; }

	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override = 0;

	void irq_out(int state);

protected:
	msx_slot_cartridge_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	devcb_write_line m_irq_handler;
	msx_cart_interface *m_cartridge;
};



class msx_cart_interface : public device_interface
{
	friend class msx_slot_cartridge_base_device;

public:
	// This is called after loading cartridge contents and allows the cartridge
	// implementation to perform some additional initialization based on the
	// cartridge contents.
	virtual std::error_condition initialize_cartridge(std::string &message) { return std::error_condition(); }
	virtual void interface_pre_start() override { assert(m_exp != nullptr); }

	void set_views(memory_view::memory_view_entry *page0, memory_view::memory_view_entry *page1, memory_view::memory_view_entry *page2, memory_view::memory_view_entry *page3);

protected:
	msx_cart_interface(const machine_config &mconfig, device_t &device);

	memory_region *cart_rom_region() { return m_exp ? m_exp->memregion("rom") : nullptr; }
	memory_region *cart_vlm5030_region() { return m_exp ? m_exp->memregion("vlm5030") : nullptr; }
	memory_region *cart_kanji_region() { return m_exp ? m_exp->memregion("kanji") : nullptr; }
	memory_region *cart_ram_region() { return m_exp ? m_exp->memregion("ram") : nullptr; }
	memory_region *cart_sram_region() { return m_exp ? m_exp->memregion("sram") : nullptr; }
	const char *get_feature(std::string_view feature_name) { return m_exp ? m_exp->get_feature(feature_name) : nullptr; }
	bool is_loaded_through_softlist() { return m_exp ? m_exp->loaded_through_softlist() : false; }
	void irq_out(int state);
	msx_slot_cartridge_base_device *parent_slot() const { return m_exp; }
	address_space &memory_space() const;
	address_space &io_space() const;
	cpu_device &maincpu() const;
	device_mixer_interface &soundin() const;
	memory_view::memory_view_entry *page(int i) { return m_page[i]; }

private:
	msx_slot_cartridge_base_device *m_exp;
	memory_view::memory_view_entry *m_page[4];
};

#endif // MAME_BUS_MSX_SLOT_CARTRIDGE_H
