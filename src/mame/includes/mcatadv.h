// license:BSD-3-Clause
// copyright-holders:Paul Priest, David Haywood
#ifndef MAME_INCLUDES_MCATADV_H
#define MAME_INCLUDES_MCATADV_H

#pragma once

#include "machine/watchdog.h"
#include "video/tmap038.h"
#include "emupal.h"
#include "tilemap.h"

class mcatadv_state : public driver_device
{
public:
	mcatadv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_spriteram(*this, "spriteram")
		, m_vidregs(*this, "vidregs")
		, m_sprdata(*this, "sprdata")
		, m_soundbank(*this, "soundbank")
		, m_maincpu(*this, "maincpu")
		, m_soundcpu(*this, "soundcpu")
		, m_watchdog(*this, "watchdog")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_tilemap(*this, "tilemap_%u", 0U)
	{ }

	void nost(machine_config &config);
	void mcatadv(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<u16> m_spriteram;
	std::unique_ptr<u16[]>   m_spriteram_old;
	required_shared_ptr<u16> m_vidregs;
	std::unique_ptr<u16[]>   m_vidregs_old;

	required_region_ptr<u8> m_sprdata;
	required_memory_bank m_soundbank;

	/* video-related */
	int m_palette_bank[2] = {};

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device_array<tilemap038_device, 2> m_tilemap;

	u16 mcat_wd_r();
	void mcatadv_sound_bw_w(u8 data);
	template<int Chip> void get_banked_color(bool tiledim, u32 &color, u32 &pri, u32 &code);
	virtual void machine_start() override;
	virtual void video_start() override;
	u32 screen_update_mcatadv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_mcatadv);
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void mcatadv_draw_tilemap_part( screen_device &screen, int layer, int i, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void mcatadv_map(address_map &map);
	void mcatadv_sound_io_map(address_map &map);
	void mcatadv_sound_map(address_map &map);
	void nost_sound_io_map(address_map &map);
	void nost_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_MCATADV_H
