// license:BSD-3-Clause
// copyright-holders:Luca Elia
#ifndef MAME_INCLUDES_REALBRK_H
#define MAME_INCLUDES_REALBRK_H

#pragma once

#include "machine/tmp68301.h"
#include "emupal.h"
#include "screen.h"

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

private:
	required_device<tmp68301_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr_array<uint16_t, 3> m_vram;
	required_shared_ptr<uint16_t> m_vregs;
	optional_shared_ptr<uint16_t> m_dsw_select;
	optional_shared_ptr<uint16_t> m_backup_ram;
	optional_shared_ptr_array<uint16_t, 2> m_vram_ras;

	optional_ioport_array<2> m_in_io;
	optional_ioport_array<4> m_dsw_io;
	optional_ioport_array<2> m_paddle_io;
	optional_ioport_array<2> m_player_io;

	std::unique_ptr<bitmap_ind16> m_tmpbitmap0;
	std::unique_ptr<bitmap_ind16> m_tmpbitmap1;
	int m_disable_video;
	tilemap_t *m_tilemap[3];

	// common
	template<int Layer> DECLARE_WRITE16_MEMBER(vram_w);
	DECLARE_WRITE16_MEMBER(vram_2_w);
	DECLARE_WRITE16_MEMBER(vregs_w);

	// realbrk and/or dai2kaku
	DECLARE_READ16_MEMBER(realbrk_dsw_r);
	DECLARE_WRITE16_MEMBER(realbrk_flipscreen_w);
	DECLARE_WRITE16_MEMBER(dai2kaku_flipscreen_w);

	// pkgnsh and/or pkgnshdx
	DECLARE_READ16_MEMBER(pkgnsh_input_r);
	DECLARE_READ16_MEMBER(pkgnshdx_input_r);
	DECLARE_READ16_MEMBER(backup_ram_r);
	DECLARE_READ16_MEMBER(backup_ram_dx_r);
	DECLARE_WRITE16_MEMBER(backup_ram_w);

	template<int Layer> TILE_GET_INFO_MEMBER(get_tile_info);
	TILE_GET_INFO_MEMBER(get_tile_info_2);

	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_dai2kaku(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	template <bool Rotatable> void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect, int layer);

	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	void base_mem(address_map &map);
	void dai2kaku_mem(address_map &map);
	void pkgnsh_mem(address_map &map);
	void pkgnshdx_mem(address_map &map);
	void realbrk_mem(address_map &map);
};

#endif // MAME_INCLUDES_REALBRK_H
