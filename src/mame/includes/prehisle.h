// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#ifndef MAME_INCLUDES_PREHISLE_H
#define MAME_INCLUDES_PREHISLE_H

#pragma once

#include "machine/gen_latch.h"
#include "sound/upd7759.h"
#include "emupal.h"
#include "tilemap.h"

class prehisle_state : public driver_device
{
public:
	prehisle_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_tx_vram(*this, "tx_vram"),
		m_spriteram(*this, "spriteram"),
		m_fg_vram(*this, "fg_vram"),
		m_tilemap_rom(*this, "bgtilemap"),
		m_io_p1(*this, "P1"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_upd7759(*this, "upd"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void prehisle(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	DECLARE_WRITE16_MEMBER(soundcmd_w);
	DECLARE_WRITE16_MEMBER(fg_vram_w);
	DECLARE_WRITE16_MEMBER(tx_vram_w);
	void fg_scrolly_w(offs_t offset, u16 data, u16 mem_mask);
	void fg_scrollx_w(offs_t offset, u16 data, u16 mem_mask);
	void bg_scrolly_w(offs_t offset, u16 data, u16 mem_mask);
	void bg_scrollx_w(offs_t offset, u16 data, u16 mem_mask);
	DECLARE_WRITE8_MEMBER(upd_port_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void prehisle_map(address_map &map);
	void prehisle_sound_io_map(address_map &map);
	void prehisle_sound_map(address_map &map);

	required_shared_ptr<uint16_t> m_tx_vram;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_fg_vram;
	required_region_ptr<uint8_t> m_tilemap_rom;

	required_ioport m_io_p1;
	uint8_t m_invert_controls;
	uint16_t m_bg_scrollx;
	uint16_t m_bg_scrolly;
	uint16_t m_fg_scrollx;
	uint16_t m_fg_scrolly;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_tx_tilemap;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<upd7759_device> m_upd7759;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
};

#endif // MAME_INCLUDES_PREHISLE_H
