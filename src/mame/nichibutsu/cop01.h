// license:BSD-3-Clause
// copyright-holders:Carlos A. Lozano
/*************************************************************************

    Cop 01

*************************************************************************/
#ifndef MAME_INCLUDES_COP01_H
#define MAME_INCLUDES_COP01_H

#pragma once

#include "machine/gen_latch.h"
#include "nb1412m2.h"
#include "emupal.h"
#include "tilemap.h"

class cop01_state : public driver_device
{
public:
	cop01_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_bgvideoram(*this, "bgvideoram"),
		m_spriteram(*this, "spriteram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void init_mightguy();
	void cop01(machine_config &config);

	/* memory pointers */
	required_shared_ptr<uint8_t> m_bgvideoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_fgvideoram;

	/* video-related */
	tilemap_t        *m_bg_tilemap = nullptr;
	tilemap_t        *m_fg_tilemap = nullptr;
	uint8_t          m_vreg[4]{};

	/* sound-related */
	int            m_pulse = 0;
	int            m_timer = 0; // kludge for ym3526 in mightguy

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	void cop01_sound_command_w(uint8_t data);
	uint8_t cop01_sound_command_r();
	void cop01_irq_ack_w(uint8_t data);
	uint8_t cop01_sound_irq_ack_w();
	void cop01_background_w(offs_t offset, uint8_t data);
	void cop01_foreground_w(offs_t offset, uint8_t data);
	void cop01_vreg_w(offs_t offset, uint8_t data);
	template <int Mask> DECLARE_READ_LINE_MEMBER(mightguy_area_r);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void cop01_palette(palette_device &palette) const;
	uint32_t screen_update_cop01(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void audio_io_map(address_map &map);
	void cop01_map(address_map &map);
	void io_map(address_map &map);
	void sound_map(address_map &map);
};

class mightguy_state : public cop01_state
{
public:
	mightguy_state(const machine_config &mconfig, device_type type, const char *tag) :
		cop01_state(mconfig, type, tag),
		m_prot(*this, "prot_chip")
	{ }

	void mightguy(machine_config &config);

private:
	void mightguy_io_map(address_map &map);
	void mightguy_audio_io_map(address_map &map);

	required_device<nb1412m2_device> m_prot;
};

#endif // MAME_INCLUDES_COP01_H
