// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_CARTRIDGE_H
#define MAME_BUS_MSX_CART_CARTRIDGE_H

#pragma once


void msx_cart(device_slot_interface &device);
void msx_yamaha_60pin(device_slot_interface &device);   // 60 pin expansion slots as found in yamaha machines


class msx_slot_cartridge_device;

class msx_cart_interface : public device_interface
{
	friend class msx_slot_cartridge_device;

public:
	// This is called after loading cartridge contents and allows the cartridge
	// implementation to perform some additional initialization based on the
	// cartridge contents.
	virtual void initialize_cartridge() { }
	virtual void interface_pre_start() override { assert(m_exp != nullptr); }

	void set_views(memory_view::memory_view_entry *page0, memory_view::memory_view_entry *page1, memory_view::memory_view_entry *page2, memory_view::memory_view_entry *page3);

	DECLARE_WRITE_LINE_MEMBER(irq_out);
	address_space &memory_space() const;
	address_space &io_space() const;
	cpu_device &maincpu() const;

	// ROM/RAM/SRAM management
	// Mainly used by the cartridge slot when loading images
	void rom_alloc(u32 size);
	void ram_alloc(u32 size);
	void rom_vlm5030_alloc(u32 size);
	void sram_alloc(u32 size);
	void kanji_alloc(u32 size);

	u8* get_rom_base() { return &m_rom[0]; }
	u8* get_rom_vlm5030_base() { return &m_rom_vlm5030[0]; }
	u8* get_ram_base() { return &m_ram[0]; }
	u8* get_sram_base() { return &m_sram[0]; }
	u8* get_kanji_base() { return &m_kanji[0]; }
	u32 get_rom_size() { return m_rom.size(); }
	u32 get_rom_vlm5030_size() { return m_rom_vlm5030.size(); }
	u32 get_ram_size() { return m_ram.size(); }
	u32 get_sram_size() { return m_sram.size(); }
	u32 get_kanji_size() { return m_kanji.size(); }
	memory_view::memory_view_entry *page(int i) { return m_page[i]; }

protected:
	msx_cart_interface(const machine_config &mconfig, device_t &device);

	std::vector<u8> m_rom;
	std::vector<u8> m_ram;
	std::vector<u8> m_rom_vlm5030;
	std::vector<u8> m_sram;
	std::vector<u8> m_kanji;
	memory_view::memory_view_entry *m_page[4];

private:
	msx_slot_cartridge_device *m_exp;
};


#endif // MAME_BUS_MSX_CART_CARTRIDGE_H
