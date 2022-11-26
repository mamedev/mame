// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_SOFTCARD_SOFTCARD_H
#define MAME_BUS_MSX_SOFTCARD_SOFTCARD_H

#pragma once


void softcard(device_slot_interface &device);


class msx_cart_softcard_device;

class softcard_interface : public device_interface
{
	friend class msx_cart_softcard_device;

public:
	virtual void initialize_cartridge() { }
	virtual void interface_pre_start() override { assert(m_exp != nullptr); }

	void set_views(memory_view::memory_view_entry *page0, memory_view::memory_view_entry *page1, memory_view::memory_view_entry *page2, memory_view::memory_view_entry *page3);

	// ROM management
	// Mainly used by the bee pack slot when loading images
	void rom_alloc(u32 size);

	u8* get_rom_base() { return &m_rom[0]; }
	u32 get_rom_size() { return m_rom.size(); }
	memory_view::memory_view_entry *page(int i) { return m_page[i]; }

protected:
	softcard_interface(const machine_config &mconfig, device_t &device);

	std::vector<u8> m_rom;
	memory_view::memory_view_entry *m_page[4];

private:
	msx_cart_softcard_device *m_exp;
};


#endif // MAME_BUS_MSX_SOFTCARD_SOFTCARD_H
