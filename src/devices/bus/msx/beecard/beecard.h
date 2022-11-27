// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_BEECARD_BEECARD_H
#define MAME_BUS_MSX_BEECARD_BEECARD_H

#pragma once


void bee_card(device_slot_interface &device);


class bee_card_interface : public device_interface
{
public:
	virtual void initialize_cartridge() { }
	void set_views(memory_view::memory_view_entry *page0, memory_view::memory_view_entry *page1, memory_view::memory_view_entry *page2, memory_view::memory_view_entry *page3);
	void rom_alloc(u32 size);
	u8* get_rom_base() { return &m_rom[0]; }
	u32 get_rom_size() { return m_rom.size(); }

protected:
	bee_card_interface(const machine_config &mconfig, device_t &device);
	memory_view::memory_view_entry *page(int i) { return m_page[i]; }

private:
	std::vector<u8> m_rom;
	memory_view::memory_view_entry *m_page[4];
};


#endif // MAME_BUS_MSX_BEECARD_BEECARD_H
