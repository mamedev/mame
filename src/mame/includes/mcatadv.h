// license:BSD-3-Clause
// copyright-holders:Paul Priest, David Haywood
#ifndef MAME_INCLUDES_MCATADV_H
#define MAME_INCLUDES_MCATADV_H

#pragma once

#include "machine/watchdog.h"
#include "emupal.h"

class mcatadv_state : public driver_device
{
public:
	mcatadv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_vram(*this, "vram_%u", 1)
		, m_scroll(*this, "scroll%u", 1)
		, m_spriteram(*this, "spriteram")
		, m_vidregs(*this, "vidregs")
		, m_sprdata(*this, "sprdata")
		, m_soundbank(*this, "soundbank")
		, m_maincpu(*this, "maincpu")
		, m_soundcpu(*this, "soundcpu")
		, m_watchdog(*this, "watchdog")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
	{ }

	void nost(machine_config &config);
	void mcatadv(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr_array<uint16_t, 2> m_vram;
	required_shared_ptr_array<uint16_t, 2> m_scroll;
	required_shared_ptr<uint16_t> m_spriteram;
	std::unique_ptr<uint16_t[]>   m_spriteram_old;
	required_shared_ptr<uint16_t> m_vidregs;
	std::unique_ptr<uint16_t[]>   m_vidregs_old;

	required_region_ptr<uint8_t> m_sprdata;
	required_memory_bank m_soundbank;

	/* video-related */
	tilemap_t    *m_tilemap[2];
	int m_palette_bank[2];

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	DECLARE_READ16_MEMBER(mcat_wd_r);
	DECLARE_WRITE8_MEMBER(mcatadv_sound_bw_w);
	template<int Chip> DECLARE_WRITE16_MEMBER(vram_w);
	template<int Chip> TILE_GET_INFO_MEMBER(get_mcatadv_tile_info);
	virtual void machine_start() override;
	virtual void video_start() override;
	uint32_t screen_update_mcatadv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
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
