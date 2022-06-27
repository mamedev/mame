// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
#ifndef MAME_INCLUDES_FCOMBAT_H
#define MAME_INCLUDES_FCOMBAT_H

#pragma once

#include "emupal.h"
#include "tilemap.h"


// this is copied from Exerion, but it should be correct
#define FCOMBAT_MASTER_CLOCK        (20000000)
#define FCOMBAT_CPU_CLOCK           (FCOMBAT_MASTER_CLOCK / 6)
#define FCOMBAT_AY8910_CLOCK        (FCOMBAT_CPU_CLOCK / 2)
#define FCOMBAT_PIXEL_CLOCK         (FCOMBAT_MASTER_CLOCK / 3)
#define FCOMBAT_HCOUNT_START        (0x58)
#define FCOMBAT_HTOTAL              (512-FCOMBAT_HCOUNT_START)
#define FCOMBAT_HBEND               (12*8)  // ??
#define FCOMBAT_HBSTART             (52*8)  //
#define FCOMBAT_VTOTAL              (256)
#define FCOMBAT_VBEND               (16)
#define FCOMBAT_VBSTART             (240)

#define BACKGROUND_X_START      32
#define BACKGROUND_X_START_FLIP 72

#define VISIBLE_X_MIN           (12*8)
#define VISIBLE_X_MAX           (52*8)
#define VISIBLE_Y_MIN           (2*8)
#define VISIBLE_Y_MAX           (30*8)


class fcombat_state : public driver_device
{
public:
	fcombat_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_bgdata_rom(*this, "bgdata"),
		m_user2_region(*this, "user2"),
		m_io_in(*this, "IN%u", 0U),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void fcombat(machine_config &config);

	void init_fcombat();

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

private:
	/* memory pointers */
	required_shared_ptr<u8> m_videoram;
	required_shared_ptr<u8> m_spriteram;
	required_region_ptr<u8> m_bgdata_rom;
	required_region_ptr<u8> m_user2_region;

	required_ioport_array<2> m_io_in;

	/* video-related */
	tilemap_t    *m_bgmap = nullptr;
	u8      m_cocktail_flip = 0U;
	u8      m_char_palette = 0U;
	u8      m_sprite_palette = 0U;
	u8      m_char_bank = 0U;

	/* misc */
	int        m_fcombat_sh = 0;
	int        m_fcombat_sv = 0;
	int        m_tx = 0;
	int        m_ty = 0;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	u8 protection_r();
	u8 port01_r();
	void e900_w(u8 data);
	void ea00_w(u8 data);
	void eb00_w(u8 data);
	void ec00_w(u8 data);
	void ed00_w(u8 data);
	u8 e300_r();
	void ee00_w(u8 data);
	void videoreg_w(u8 data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void fcombat_palette(palette_device &palette) const;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void audio_map(address_map &map);
	void main_map(address_map &map);
};

#endif // MAME_INCLUDES_FCOMBAT_H
