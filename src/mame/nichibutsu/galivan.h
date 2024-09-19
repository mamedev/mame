// license:BSD-3-Clause
// copyright-holders:Luca Elia, Olivier Galibert
/***************************************************************************

    Galivan - Cosmo Police

***************************************************************************/
#ifndef MAME_NICHIBUTSU_GALIVAN_H
#define MAME_NICHIBUTSU_GALIVAN_H

#pragma once

#include "nb1412m2.h"
#include "nb1414m4.h"

#include "machine/gen_latch.h"
#include "video/bufsprite.h"
#include "sound/flt_biquad.h"

#include "screen.h"
#include "emupal.h"
#include "tilemap.h"


class galivan_state : public driver_device
{
public:
	galivan_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_videoram(*this, "videoram")
		, m_spriteram(*this, "spriteram")
		, m_nb1414m4(*this, "nb1414m4")
		, m_screen(*this, "screen")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
		, m_dacfilter1(*this, "dacfilter1")
		, m_dacfilter2(*this, "dacfilter2")
		, m_ymfilter(*this, "ymfilter")
		, m_rombank(*this, "rombank")
	{ }

	void galivan(machine_config &config);
	void ninjemak(machine_config &config);
	void youmab(machine_config &config);

	void init_youmab();

protected:
	void io_map(address_map &map) ATTR_COLD;

	void galivan_common(machine_config &config);
	void video_config(machine_config &config);

	required_device<cpu_device> m_maincpu;

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_device<buffered_spriteram8_device> m_spriteram;

	/* video-related */
	tilemap_t     *m_bg_tilemap = nullptr;
	tilemap_t     *m_tx_tilemap = nullptr;
	uint16_t       m_scrollx = 0U;
	uint16_t       m_scrolly = 0U;
	uint8_t        m_galivan_scrollx[2]{}, m_galivan_scrolly[2]{};
	uint8_t        m_layers = 0U;
	uint8_t        m_ninjemak_dispdisable = 0U;

	uint8_t        m_shift_scroll = 0U; //youmab
	uint32_t       m_shift_val = 0U;
	void galivan_sound_command_w(uint8_t data);
	uint8_t soundlatch_clear_r();
	uint8_t IO_port_c0_r();
	void blit_trigger_w(uint8_t data);
	void vblank_ack_w(uint8_t data);
	void youmab_extra_bank_w(uint8_t data);
	uint8_t youmab_8a_r();
	void youmab_81_w(uint8_t data);
	void youmab_84_w(uint8_t data);
	void youmab_86_w(uint8_t data);
	void galivan_videoram_w(offs_t offset, uint8_t data);
	void galivan_gfxbank_w(uint8_t data);
	void ninjemak_gfxbank_w(uint8_t data);
	void galivan_scrollx_w(offs_t offset, uint8_t data);
	void galivan_scrolly_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	TILE_GET_INFO_MEMBER(ninjemak_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(ninjemak_get_tx_tile_info);
	void galivan_palette(palette_device &palette) const;
	void ninjemak_palette(palette_device &palette) const;
	DECLARE_MACHINE_START(galivan);
	DECLARE_MACHINE_RESET(galivan);
	DECLARE_VIDEO_START(galivan);
	DECLARE_MACHINE_START(ninjemak);
	DECLARE_MACHINE_RESET(ninjemak);
	DECLARE_VIDEO_START(ninjemak);
	uint32_t screen_update_galivan(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_ninjemak(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );

	optional_device<nb1414m4_device> m_nb1414m4;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<filter_biquad_device> m_dacfilter1;
	required_device<filter_biquad_device> m_dacfilter2;
	required_device<filter_biquad_device> m_ymfilter;
	memory_bank_creator m_rombank;

	void galivan_map(address_map &map) ATTR_COLD;
	void ninjemak_io_map(address_map &map) ATTR_COLD;
	void ninjemak_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};

class dangarj_state : public galivan_state
{
public:
	dangarj_state(const machine_config &mconfig, device_type type, const char *tag) :
		galivan_state(mconfig, type, tag),
		m_prot(*this, "prot_chip")
	{ }

	void dangarj(machine_config &config);

private:
	required_device<nb1412m2_device> m_prot;

	void dangarj_io_map(address_map &map) ATTR_COLD;
};

#endif // MAME_NICHIBUTSU_GALIVAN_H
