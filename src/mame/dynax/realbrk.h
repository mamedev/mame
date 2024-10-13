// license:BSD-3-Clause
// copyright-holders:Luca Elia
#ifndef MAME_DYNAX_REALBRK_H
#define MAME_DYNAX_REALBRK_H

#pragma once

#include "cpu/m68000/tmp68301.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class realbrk_state : public driver_device
{
public:
	realbrk_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_vram(*this, "vram_%u", 0U),
		m_vregs(*this, "vregs"),
		m_dsw_select(*this, "dsw_select"),
		m_backup_ram(*this, "backup_ram"),
		m_vram_ras(*this, "vram_%uras", 0U),
		m_in_io(*this, "IN%u", 0U),
		m_dsw_io(*this, "SW%u", 1U),
		m_paddle_io(*this, "PADDLE%u", 1U),
		m_player_io(*this, "P%u", 1U)
	{ }

	void pkgnsh(machine_config &config);
	void dai2kaku(machine_config &config);
	void realbrk(machine_config &config);
	void pkgnshdx(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<tmp68301_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<u16> m_spriteram;
	required_shared_ptr_array<u16, 3> m_vram;
	required_shared_ptr<u16> m_vregs;
	optional_shared_ptr<u16> m_dsw_select;
	optional_shared_ptr<u16> m_backup_ram;
	optional_shared_ptr_array<u16, 2> m_vram_ras;

	optional_ioport_array<2> m_in_io;
	optional_ioport_array<4> m_dsw_io;
	optional_ioport_array<2> m_paddle_io;
	optional_ioport_array<2> m_player_io;

	std::unique_ptr<bitmap_ind16> m_tmpbitmap0;
	std::unique_ptr<bitmap_ind16> m_tmpbitmap1;
	int m_disable_video = 0;
	tilemap_t *m_tilemap[3]{};

	// common
	template<int Layer> void vram_w(offs_t offset, u16 data, u16 mem_mask);
	void vram_2_w(offs_t offset, u16 data, u16 mem_mask);
	void vregs_w(offs_t offset, u16 data, u16 mem_mask);

	// realbrk and/or dai2kaku
	u16 realbrk_dsw_r();
	void realbrk_flipscreen_w(offs_t offset, u16 data, u16 mem_mask);
	void dai2kaku_flipscreen_w(u16 data);

	// pkgnsh and/or pkgnshdx
	u16 pkgnsh_input_r(offs_t offset);
	u16 pkgnshdx_input_r(offs_t offset);
	u16 backup_ram_r(offs_t offset);
	u16 backup_ram_dx_r(offs_t offset);
	void backup_ram_w(offs_t offset, u16 data, u16 mem_mask);

	template<int Layer> TILE_GET_INFO_MEMBER(get_tile_info);
	TILE_GET_INFO_MEMBER(get_tile_info_2);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_dai2kaku(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	template <bool Rotatable> void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect, bitmap_ind8 &priority);

	void vblank_irq(int state);
	void base_mem(address_map &map) ATTR_COLD;
	void dai2kaku_mem(address_map &map) ATTR_COLD;
	void pkgnsh_mem(address_map &map) ATTR_COLD;
	void pkgnshdx_mem(address_map &map) ATTR_COLD;
	void realbrk_mem(address_map &map) ATTR_COLD;
};

#endif // MAME_DYNAX_REALBRK_H
