// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_TVGAMES_ELAN_EU3A05VID_H
#define MAME_TVGAMES_ELAN_EU3A05VID_H

#include "elan_eu3a05commonvid.h"
#include "cpu/m6502/m6502.h"
#include "machine/bankdev.h"
#include "screen.h"

class elan_eu3a05vid_device : public elan_eu3a05commonvid_device, public device_memory_interface
{
public:
	elan_eu3a05vid_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_cpu(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_addrbank(T &&tag) { m_bank.set_tag(std::forward<T>(tag)); }

	void map(address_map &map) ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void set_is_sudoku();
	void set_is_pvmilfin();
	void set_use_spritepages() { m_use_spritepages = true; }
	void set_force_basic_scroll() { m_force_basic_scroll = true; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

private:
	required_device<m6502_device> m_cpu;
	required_device<address_map_bank_device> m_bank;
	const address_space_config      m_space_config;

	uint8_t m_vidctrl = 0;

	uint8_t m_tile_gfxbase_lo_data = 0;
	uint8_t m_tile_gfxbase_hi_data = 0;

	uint8_t m_sprite_gfxbase_lo_data = 0;
	uint8_t m_sprite_gfxbase_hi_data = 0;

	uint8_t m_tile_scroll[4*2]{};

	uint8_t m_splitpos[2]{};
	uint8_t m_transpen = 0;

	uint16_t get_scroll(int which);

	bool get_tile_data(int base, int drawpri, int& tile, int &attr, int &unk2);
	void draw_tilemaps(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int drawpri);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, bitmap_ind8 &priority_bitmap, const rectangle &cliprect);
	void draw_tilemaps_tileline(int drawpri, int tile, int attr, int unk2, int tilexsize, int tileline, int xpos, uint16_t *row);
	uint16_t get_tilemapindex_from_xy(uint16_t x, uint16_t y);

	uint8_t read_spriteram(int offset);
	uint8_t read_vram(int offset);

	// VIDEO
	// tile bases
	void tile_gfxbase_lo_w(uint8_t data);
	void tile_gfxbase_hi_w(uint8_t data);
	uint8_t tile_gfxbase_lo_r();
	uint8_t tile_gfxbase_hi_r();
	// sprite tile bases
	void sprite_gfxbase_lo_w(uint8_t data);
	void sprite_gfxbase_hi_w(uint8_t data);
	uint8_t sprite_gfxbase_lo_r();
	uint8_t sprite_gfxbase_hi_r();

	uint8_t transpen_r();
	void transpen_w(uint8_t data);

	uint8_t elan_eu3a05_vidctrl_r();
	void elan_eu3a05_vidctrl_w(uint8_t data);

	uint8_t tile_scroll_r(offs_t offset);
	void tile_scroll_w(offs_t offset, uint8_t data);

	uint8_t splitpos_r(offs_t offset);
	void splitpos_w(offs_t offset, uint8_t data);

	uint8_t read_unmapped(offs_t offset);
	void write_unmapped(offs_t offset, uint8_t data);

	int m_bytes_per_tile_entry;
	int m_vrambase;
	int m_spritebase;
	bool m_use_spritepages;
	bool m_force_basic_scroll;
};

DECLARE_DEVICE_TYPE(ELAN_EU3A05_VID, elan_eu3a05vid_device)

#endif // MAME_TVGAMES_ELAN_EU3A05VID_H
