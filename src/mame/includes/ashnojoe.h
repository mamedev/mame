// license:BSD-3-Clause
// copyright-holders:David Haywood

/*************************************************************************

    Success Joe / Ashita no Joe

*************************************************************************/
#ifndef MAME_INCLUDES_ASHNOJOE_H
#define MAME_INCLUDES_ASHNOJOE_H

#pragma once

#include "machine/gen_latch.h"
#include "sound/msm5205.h"

class ashnojoe_state : public driver_device
{
public:
	ashnojoe_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_tileram(*this, "tileram_%u", 1U),
		m_tilemap_reg(*this, "tilemap_reg"),
		m_audiobank(*this, "audiobank"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void init_ashnojoe();

	void ashnojoe(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	/* memory pointers */
	required_shared_ptr_array<u16, 7> m_tileram;
	required_shared_ptr<u16> m_tilemap_reg;
	required_memory_bank m_audiobank;

	/* video-related */
	tilemap_t *m_tilemap[7];

	/* sound-related */
	u8        m_adpcm_byte;
	int       m_msm5205_vclk_toggle;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<generic_latch_8_device> m_soundlatch;

	u16 fake_4a00a_r();
	void adpcm_w(u8 data);
	u8 sound_latch_status_r();
	template<unsigned Which> void tileram_8x8_w(offs_t offset, u16 data);
	template<unsigned Which> void tileram_16x16_w(offs_t offset, u16 data);
	void tilemaps_xscroll_w(offs_t offset, u16 data);
	void tilemaps_yscroll_w(offs_t offset, u16 data);
	void tilemap_regs_w(offs_t offset, u16 data, u16 mem_mask);
	void ym2203_write_a(u8 data);
	void ym2203_write_b(u8 data);
	TILE_GET_INFO_MEMBER(get_tile_info_highest);
	TILE_GET_INFO_MEMBER(get_tile_info_midlow);
	TILE_GET_INFO_MEMBER(get_tile_info_high);
	TILE_GET_INFO_MEMBER(get_tile_info_low);
	TILE_GET_INFO_MEMBER(get_tile_info_midhigh);
	TILE_GET_INFO_MEMBER(get_tile_info_lowest);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(ashnojoe_vclk_cb);
	void ashnojoe_map(address_map &map);
	void sound_map(address_map &map);
	void sound_portmap(address_map &map);
};

#endif // MAME_INCLUDES_ASHNOJOE_H
