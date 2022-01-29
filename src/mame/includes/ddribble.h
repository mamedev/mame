// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

    Double Dribble

***************************************************************************/
#ifndef MAME_INCLUDES_DDRIBBLE_H
#define MAME_INCLUDES_DDRIBBLE_H

#pragma once

#include "sound/flt_rc.h"
#include "sound/vlm5030.h"
#include "emupal.h"
#include "tilemap.h"

class ddribble_state : public driver_device
{
public:
	ddribble_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_fg_videoram(*this, "fg_videoram"),
		m_spriteram(*this, "spriteram_%u", 1U),
		m_bg_videoram(*this, "bg_videoram"),
		m_mainbank(*this, "mainbank"),
		m_vlmbank(*this, "vlmbank"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_vlm(*this, "vlm"),
		m_filter(*this, "filter%u", 1U),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void ddribble(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_fg_videoram;
	required_shared_ptr_array<uint8_t, 2> m_spriteram;
	required_shared_ptr<uint8_t> m_bg_videoram;
	required_memory_bank m_mainbank;
	required_memory_bank m_vlmbank;

	// video-related
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	uint8_t m_vregs[2][5];
	uint8_t m_charbank[2];

	// misc
	uint8_t  m_int_enable[2];

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<vlm5030_device> m_vlm;
	required_device_array<filter_rc_device, 3> m_filter;
	required_device<gfxdecode_device> m_gfxdecode;

	void bankswitch_w(uint8_t data);
	void coin_counter_w(uint8_t data);
	void K005885_0_w(offs_t offset, uint8_t data);
	void K005885_1_w(offs_t offset, uint8_t data);
	void fg_videoram_w(offs_t offset, uint8_t data);
	void bg_videoram_w(offs_t offset, uint8_t data);
	uint8_t vlm5030_busy_r();
	void vlm5030_ctrl_w(uint8_t data);
	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t* source, int lenght, int gfxset, int flipscreen);
	void maincpu_map(address_map &map);
	void subcpu_map(address_map &map);
	void audiocpu_map(address_map &map);
	void vlm_map(address_map &map);
};

#endif // MAME_INCLUDES_DDRIBBLE_H
