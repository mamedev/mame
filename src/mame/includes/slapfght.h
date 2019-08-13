// license:BSD-3-Clause
// copyright-holders:K.Wilkins,Stephane Humbert
/***************************************************************************

  Toaplan Slap Fight hardware

***************************************************************************/
#ifndef MAME_INCLUDES_SLAPFGHT_H
#define MAME_INCLUDES_SLAPFGHT_H

#pragma once

#include "cpu/z80/z80.h"
#include "video/bufsprite.h"
#include "machine/taito68705interface.h"
#include "emupal.h"
#include "screen.h"

class slapfght_state : public driver_device
{
public:
	slapfght_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_bmcu(*this, "bmcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_fixvideoram(*this, "fixvideoram"),
		m_fixcolorram(*this, "fixcolorram")
	{ }

	void tigerh(machine_config &config);
	void tigerhb1(machine_config &config);
	void tigerhb2(machine_config &config);
	void tigerhb4(machine_config &config);
	void getstarb2(machine_config &config);
	void slapfighb2(machine_config &config);
	void getstarb1(machine_config &config);
	void perfrman(machine_config &config);
	void slapfigh(machine_config &config);
	void slapfighb1(machine_config &config);

	void init_banks();
	void init_getstarb1();
	void init_slapfigh();
	void init_getstarb2();

private:
	// devices, memory pointers
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<taito68705_mcu_device> m_bmcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<buffered_spriteram8_device> m_spriteram;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	optional_shared_ptr<uint8_t> m_fixvideoram;
	optional_shared_ptr<uint8_t> m_fixcolorram;

	/* This it the best way to allow game specific kludges until the system is fully understood */
	enum getstar_id
	{
		GETSTUNK = 0, /* unknown for inclusion of possible new sets */
		//GETSTAR,
		//GETSTARJ,
		GETSTARB1,    /* "good" bootleg with same behaviour as 'getstarj' */
		GETSTARB2     /* "lame" bootleg with lots of ingame bugs */
	} m_getstar_id;

	tilemap_t *m_pf1_tilemap;
	tilemap_t *m_fix_tilemap;
	uint8_t m_palette_bank;
	uint8_t m_scrollx_lo;
	uint8_t m_scrollx_hi;
	uint8_t m_scrolly;
	bool m_main_irq_enabled;
	bool m_sound_nmi_enabled;

	int m_getstar_status;
	int m_getstar_sequence_index;
	int m_getstar_status_state;
	uint8_t m_getstar_cmd;
	uint8_t m_gs_a;
	uint8_t m_gs_d;
	uint8_t m_gs_e;
	uint8_t m_tigerhb_cmd;

	DECLARE_READ8_MEMBER(tigerh_mcu_status_r);
	DECLARE_WRITE_LINE_MEMBER(sound_reset_w);
	DECLARE_WRITE_LINE_MEMBER(irq_enable_w);
	DECLARE_READ8_MEMBER(vblank_r);
	DECLARE_WRITE8_MEMBER(sound_nmi_enable_w);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(colorram_w);
	DECLARE_WRITE8_MEMBER(fixram_w);
	DECLARE_WRITE8_MEMBER(fixcol_w);
	DECLARE_WRITE8_MEMBER(scrollx_lo_w);
	DECLARE_WRITE8_MEMBER(scrollx_hi_w);
	DECLARE_WRITE8_MEMBER(scrolly_w);
	DECLARE_WRITE_LINE_MEMBER(flipscreen_w);
	DECLARE_WRITE_LINE_MEMBER(palette_bank_w);

	DECLARE_WRITE8_MEMBER(scroll_from_mcu_w);

	DECLARE_READ8_MEMBER(getstar_mcusim_r);
	DECLARE_WRITE8_MEMBER(getstar_mcusim_w);
	DECLARE_READ8_MEMBER(getstar_mcusim_status_r);
	DECLARE_READ8_MEMBER(getstarb1_prot_r);
	DECLARE_READ8_MEMBER(tigerhb1_prot_r);
	DECLARE_WRITE8_MEMBER(tigerhb1_prot_w);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	TILE_GET_INFO_MEMBER(get_pf_tile_info);
	TILE_GET_INFO_MEMBER(get_pf1_tile_info);
	TILE_GET_INFO_MEMBER(get_fix_tile_info);
	DECLARE_VIDEO_START(perfrman);
	DECLARE_VIDEO_START(slapfight);

	void draw_perfrman_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int layer);
	void draw_slapfight_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_perfrman(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_slapfight(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	INTERRUPT_GEN_MEMBER(sound_nmi);

	void getstar_map(address_map &map);
	void getstarb1_io_map(address_map &map);
	void getstarb2_io_map(address_map &map);
	void io_map_mcu(address_map &map);
	void io_map_nomcu(address_map &map);
	void perfrman_map(address_map &map);
	void perfrman_sound_map(address_map &map);
	void slapfigh_map(address_map &map);
	void slapfigh_map_mcu(address_map &map);
	void slapfighb1_map(address_map &map);
	void slapfighb2_map(address_map &map);
	void tigerh_map(address_map &map);
	void tigerh_map_mcu(address_map &map);
	void tigerh_sound_map(address_map &map);
	void tigerhb1_map(address_map &map);
	void tigerhb2_map(address_map &map);
};

#endif // MAME_INCLUDES_SLAPFGHT_H
