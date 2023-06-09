// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Nicola Salmoria
#ifndef MAME_MISC_HOMEDATA_H
#define MAME_MISC_HOMEDATA_H

#pragma once

#include "machine/gen_latch.h"
#include "sound/sn76496.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "tilemap.h"


class homedata_state : public driver_device
{
public:
	homedata_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_vreg(*this, "vreg"),
		m_videoram(*this, "videoram"),
		m_blit_rom(*this, "blit_rom"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_ymsnd(*this, "ymsnd"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_mainlatch(*this, "mainlatch"),
		m_sn(*this, "snsnd"),
		m_keys(*this, "KEY%u", 0U)
	{
	}

	void mirderby(machine_config &config) ATTR_COLD;

	void init_mirderby() ATTR_COLD;

protected:
	/* memory pointers */
	optional_shared_ptr<uint8_t> m_vreg;
	required_shared_ptr<uint8_t> m_videoram;

	optional_region_ptr<uint8_t> m_blit_rom;

	/* video-related */
	tilemap_t *m_bg_tilemap[2][4]{};
	int      m_visible_page = 0;
	int      m_priority = 0;
	int      m_flipscreen = 0;
	uint8_t  m_blitter_bank = 0;
	int      m_blitter_param_count = 0;
	uint8_t  m_blitter_param[4]{};      /* buffers last 4 writes to 0x8006 */


	/* misc */
	int      m_vblank = 0;
	int      m_keyb = 0;

	/* device */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<ym2203_device> m_ymsnd;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch;
	optional_device<generic_latch_8_device> m_mainlatch; // pteacher
	optional_device<sn76489a_device> m_sn; // mrokumei and pteacher

	optional_ioport_array<12> m_keys;

	uint8_t m_prot_data = 0;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t mirderby_prot_r();
	void mirderby_prot_w(uint8_t data);
	void mrokumei_videoram_w(offs_t offset, u8 data);
	void homedata_blitter_param_w(uint8_t data);
	TILE_GET_INFO_MEMBER(mirderby_get_info0_0);
	TILE_GET_INFO_MEMBER(mirderby_get_info1_0);
	TILE_GET_INFO_MEMBER(mirderby_get_info0_1);
	TILE_GET_INFO_MEMBER(mirderby_get_info1_1);
	DECLARE_VIDEO_START(mirderby);
	void mirderby_palette(palette_device &palette) const ATTR_COLD;
	uint32_t screen_update_mirderby(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	INTERRUPT_GEN_MEMBER(homedata_irq);
	void mrokumei_handleblit(int rom_base);
	void mirderby_info0(tile_data &tileinfo, int tile_index, int page, int gfxbank);
	void mirderby_info1(tile_data &tileinfo, int tile_index, int page, int gfxbank);

	void cpu0_map(address_map &map) ATTR_COLD;
	void cpu1_map(address_map &map) ATTR_COLD;
	void cpu2_map(address_map &map) ATTR_COLD;
};


class mrokumei_state : public homedata_state
{
public:
	mrokumei_state(const machine_config &mconfig, device_type type, const char *tag) :
		homedata_state(mconfig, type, tag),
		m_audiobank(*this, "audiobank"),
		m_soundview(*this, "soudview")
	{
	}

	void mrokumei(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_memory_bank m_audiobank;
	memory_view m_soundview;

	uint8_t mrokumei_keyboard_r(offs_t offset);
	void mrokumei_keyboard_select_w(uint8_t data);
	void mrokumei_sound_bank_w(uint8_t data);
	void mrokumei_sound_cmd_w(uint8_t data);
	void mrokumei_blitter_bank_w(uint8_t data);
	void mrokumei_blitter_start_w(uint8_t data);

	void mrokumei_palette(palette_device &palette) const ATTR_COLD;

	void mrokumei_info0(tile_data &tileinfo, int tile_index, int page, int gfxbank);
	void mrokumei_info1(tile_data &tileinfo, int tile_index, int page, int gfxbank);
	TILE_GET_INFO_MEMBER(mrokumei_get_info0_0);
	TILE_GET_INFO_MEMBER(mrokumei_get_info1_0);
	TILE_GET_INFO_MEMBER(mrokumei_get_info0_1);
	TILE_GET_INFO_MEMBER(mrokumei_get_info1_1);

	uint32_t screen_update_mrokumei(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void mrokumei_map(address_map &map) ATTR_COLD;
	void mrokumei_sound_map(address_map &map) ATTR_COLD;
	void mrokumei_sound_io_map(address_map &map) ATTR_COLD;
};


class homedata_upd7807_state : public homedata_state
{
public:
	homedata_upd7807_state(const machine_config &mconfig, device_type type, const char *tag) :
		homedata_state(mconfig, type, tag),
		m_mainrom(*this, "maincpu"),
		m_mainbank(*this, "mainbank"),
		m_audiobank(*this, "audiobank")
	{
	}

	void reikaids(machine_config &config) ATTR_COLD;
	void pteacher(machine_config &config) ATTR_COLD;
	void jogakuen(machine_config &config) ATTR_COLD;
	void mjkinjas(machine_config &config) ATTR_COLD;
	void lemnangl(machine_config &config) ATTR_COLD;
	void mjikaga(machine_config &config) ATTR_COLD;

	void init_reikaids() ATTR_COLD;
	void init_battlcry() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_region_ptr<uint8_t> m_mainrom;
	required_memory_bank m_mainbank;
	required_memory_bank m_audiobank;

	// video-related
	uint8_t  m_reikaids_which = 0;
	uint8_t  m_gfx_bank[2]{};   // pteacher only uses the first one

	// misc
	uint8_t  m_upd7807_porta = 0;
	uint8_t  m_upd7807_portc = 0;

	uint8_t reikaids_upd7807_porta_r();
	void reikaids_upd7807_porta_w(uint8_t data);
	void reikaids_upd7807_portc_w(uint8_t data);
	uint8_t reikaids_io_r();
	uint8_t pteacher_io_r();
	uint8_t pteacher_keyboard_r();
	uint8_t pteacher_upd7807_porta_r();
	void pteacher_upd7807_porta_w(uint8_t data);
	void pteacher_upd7807_portc_w(uint8_t data);
	void bankswitch_w(uint8_t data);
	void reikaids_videoram_w(offs_t offset, u8 data);
	void reikaids_gfx_bank_w(uint8_t data);
	void pteacher_gfx_bank_w(uint8_t data);
	void reikaids_blitter_bank_w(uint8_t data);
	void pteacher_blitter_bank_w(uint8_t data);
	void reikaids_blitter_start_w(uint8_t data);
	void pteacher_blitter_start_w(uint8_t data);

	void reikaids_handleblit(int rom_base);
	void pteacher_handleblit(int rom_base);

	void reikaids_palette(palette_device &palette) const ATTR_COLD;
	void pteacher_palette(palette_device &palette) const ATTR_COLD;

	void reikaids_info(tile_data &tileinfo, int tile_index, int page, int layer, int gfxbank);
	TILE_GET_INFO_MEMBER(reikaids_get_info0_0);
	TILE_GET_INFO_MEMBER(reikaids_get_info1_0);
	TILE_GET_INFO_MEMBER(reikaids_get_info0_1);
	TILE_GET_INFO_MEMBER(reikaids_get_info1_1);
	TILE_GET_INFO_MEMBER(reikaids_get_info0_2);
	TILE_GET_INFO_MEMBER(reikaids_get_info1_2);
	TILE_GET_INFO_MEMBER(reikaids_get_info0_3);
	TILE_GET_INFO_MEMBER(reikaids_get_info1_3);

	void pteacher_info(tile_data &tileinfo, int tile_index, int page, int layer, int gfxbank);
	TILE_GET_INFO_MEMBER(pteacher_get_info0_0);
	TILE_GET_INFO_MEMBER(pteacher_get_info1_0);
	TILE_GET_INFO_MEMBER(pteacher_get_info0_1);
	TILE_GET_INFO_MEMBER(pteacher_get_info1_1);

	void lemnangl_info(tile_data &tileinfo, int tile_index, int page, int layer, int gfxset, int gfxbank);
	TILE_GET_INFO_MEMBER(lemnangl_get_info0_0);
	TILE_GET_INFO_MEMBER(lemnangl_get_info1_0);
	TILE_GET_INFO_MEMBER(lemnangl_get_info0_1);
	TILE_GET_INFO_MEMBER(lemnangl_get_info1_1);

	uint32_t screen_update_reikaids(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_pteacher(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_MACHINE_START(reikaids);
	DECLARE_MACHINE_RESET(reikaids);
	DECLARE_MACHINE_RESET(pteacher);
	DECLARE_VIDEO_START(reikaids);
	DECLARE_VIDEO_START(pteacher);
	DECLARE_VIDEO_START(lemnangl);

	void reikaids_map(address_map &map) ATTR_COLD;
	void reikaids_upd7807_map(address_map &map) ATTR_COLD;
	void pteacher_base_map(address_map &map) ATTR_COLD;
	void pteacher_map(address_map &map) ATTR_COLD;
	void pteacher_upd7807_map(address_map &map) ATTR_COLD;
	void jogakuen_map(address_map &map) ATTR_COLD;
	void mjikaga_map(address_map &map) ATTR_COLD;
	void mjikaga_upd7807_map(address_map &map) ATTR_COLD;
};

#endif // MAME_MISC_HOMEDATA_H
