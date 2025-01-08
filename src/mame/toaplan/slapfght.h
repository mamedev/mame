// license:BSD-3-Clause
// copyright-holders:K.Wilkins,Stephane Humbert
/***************************************************************************

  Toaplan Slap Fight hardware

***************************************************************************/
#ifndef MAME_TOAPLAN_SLAPFGHT_H
#define MAME_TOAPLAN_SLAPFGHT_H

#pragma once

#include "taito68705.h"

#include "cpu/z80/z80.h"
#include "video/bufsprite.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

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
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_spriteram_buffer(*this, "spriteram"),
		m_fixvideoram(*this, "fixvideoram"),
		m_fixcolorram(*this, "fixcolorram")
	{ }

	void tigerh(machine_config &config);
	void tigerhb1(machine_config &config);
	void tigerhb2(machine_config &config);
	void tigerhb4(machine_config &config);
	void getstarb2(machine_config &config);
	void getstarb1(machine_config &config);
	void perfrman(machine_config &config);
	void slapfigh(machine_config &config);
	void slapfigha(machine_config &config);
	void slapfighb1(machine_config &config);
	void slapfighb2(machine_config &config);

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

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_device<buffered_spriteram8_device> m_spriteram_buffer;
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

	tilemap_t *m_pf1_tilemap = nullptr;
	tilemap_t *m_fix_tilemap = nullptr;
	uint8_t m_palette_bank = 0;
	uint8_t m_scrollx_lo = 0;
	uint8_t m_scrollx_hi = 0;
	uint8_t m_scrolly = 0;
	bool m_main_irq_enabled = false;
	bool m_sound_nmi_enabled = false;

	int m_getstar_status = 0;
	int m_getstar_sequence_index = 0;
	int m_getstar_status_state = 0;
	uint8_t m_getstar_cmd = 0;
	uint8_t m_gs_a = 0;
	uint8_t m_gs_d = 0;
	uint8_t m_gs_e = 0;
	uint8_t m_tigerhb_cmd = 0;

	uint8_t tigerh_mcu_status_r();
	void sound_reset_w(int state);
	void irq_enable_w(int state);
	uint8_t vblank_r();
	void sound_nmi_enable_w(offs_t offset, uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	void fixram_w(offs_t offset, uint8_t data);
	void fixcol_w(offs_t offset, uint8_t data);
	void scrollx_lo_w(uint8_t data);
	void scrollx_hi_w(uint8_t data);
	void scrolly_w(uint8_t data);
	void flipscreen_w(int state);
	void palette_bank_w(int state);

	void scroll_from_mcu_w(offs_t offset, uint8_t data);

	uint8_t getstar_mcusim_r();
	void getstar_mcusim_w(uint8_t data);
	uint8_t getstar_mcusim_status_r();
	uint8_t getstarb1_prot_r();
	uint8_t tigerhb1_prot_r();
	void tigerhb1_prot_w(uint8_t data);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	TILE_GET_INFO_MEMBER(get_pf_tile_info);
	TILE_GET_INFO_MEMBER(get_pf1_tile_info);
	TILE_GET_INFO_MEMBER(get_fix_tile_info);
	DECLARE_VIDEO_START(perfrman);
	DECLARE_VIDEO_START(slapfight);

	void draw_perfrman_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int layer);
	void draw_slapfight_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_perfrman(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_slapfight(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void vblank_irq(int state);
	INTERRUPT_GEN_MEMBER(sound_nmi);

	void getstar_map(address_map &map) ATTR_COLD;
	void getstarb1_io_map(address_map &map) ATTR_COLD;
	void getstarb2_io_map(address_map &map) ATTR_COLD;
	void io_map_mcu(address_map &map) ATTR_COLD;
	void io_map_nomcu(address_map &map) ATTR_COLD;
	void perfrman_map(address_map &map) ATTR_COLD;
	void perfrman_sound_map(address_map &map) ATTR_COLD;
	void slapfigh_map(address_map &map) ATTR_COLD;
	void slapfigh_map_mcu(address_map &map) ATTR_COLD;
	void slapfighb1_map(address_map &map) ATTR_COLD;
	void slapfighb2_map(address_map &map) ATTR_COLD;
	void tigerh_map(address_map &map) ATTR_COLD;
	void tigerh_map_mcu(address_map &map) ATTR_COLD;
	void tigerh_sound_map(address_map &map) ATTR_COLD;
	void tigerhb1_map(address_map &map) ATTR_COLD;
	void tigerhb2_map(address_map &map) ATTR_COLD;
};

#endif // MAME_TOAPLAN_SLAPFGHT_H
