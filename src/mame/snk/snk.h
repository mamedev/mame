// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi,Tim Lindquist,Carlos A. Lozano,Bryan McPhail,Jarek Parchanski,Nicola Salmoria,Tomasz Slanina,Phil Stroffolino,Acho A. Tang,Victor Trucco
// thanks-to:Marco Cassili

/*************************************************************************

    various SNK triple Z80 games

*************************************************************************/
#ifndef MAME_SNK_SNK_H
#define MAME_SNK_SNK_H

#pragma once

#include "machine/gen_latch.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class snk_state : public driver_device
{
public:
	snk_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_spriteram(*this, "spriteram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_tx_videoram(*this, "tx_videoram"),
		m_bonus_io(*this, "BONUS")
	{ }

	void fitegolf(machine_config &config) ATTR_COLD;
	void aso(machine_config &config) ATTR_COLD;
	void fitegolf2(machine_config &config) ATTR_COLD;
	void jcross(machine_config &config) ATTR_COLD;
	void tnk3(machine_config &config) ATTR_COLD;
	void hal21(machine_config &config) ATTR_COLD;
	void athena(machine_config &config) ATTR_COLD;
	void sgladiat(machine_config &config) ATTR_COLD;

	int sound_busy_r();
	template <int Mask> ioport_value snk_bonus_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_bg_videoram;
	required_shared_ptr<uint8_t> m_tx_videoram;

	optional_ioport m_bonus_io;

	uint8_t m_sound_status = 0;

	tilemap_t *m_tx_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;
	int32_t m_bg_scrollx = 0;
	int32_t m_bg_scrolly = 0;
	int32_t m_sp16_scrollx = 0;
	int32_t m_sp16_scrolly = 0;
	int32_t m_sp32_scrollx = 0;
	int32_t m_sp32_scrolly = 0;
	uint8_t m_sprite_split_point = 0;

	int m_num_sprites = 0;
	int m_yscroll_mask = 0;
	uint32_t m_bg_tile_offset = 0;
	uint32_t m_tx_tile_offset = 0;

	uint8_t m_drawmode_table[16]{};

	uint8_t cpuA_nmi_trigger_r();
	void cpuA_nmi_ack_w(uint8_t data);
	uint8_t cpuB_nmi_trigger_r();
	void cpuB_nmi_ack_w(uint8_t data);
	void sgladiat_soundlatch_w(uint8_t data);
	uint8_t sgladiat_soundlatch_r();
	uint8_t sgladiat_sound_nmi_ack_r();
	uint8_t sgladiat_sound_irq_ack_r();
	void soundlatch_w(uint8_t data);
	uint8_t sound_status_r();
	void sound_status_w(uint8_t data);
	uint8_t tnk3_cmdirq_ack_r();
	uint8_t tnk3_ymirq_ack_r();
	uint8_t tnk3_busy_clear_r();
	void athena_coin_counter_w(uint8_t data);
	void ikari_coin_counter_w(uint8_t data);
	void tdfever_coin_counter_w(uint8_t data);
	void tx_videoram_w(offs_t offset, uint8_t data);
	void marvins_bg_videoram_w(offs_t offset, uint8_t data);
	void bg_videoram_w(offs_t offset, uint8_t data);
	void bg_scrollx_w(uint8_t data);
	void bg_scrolly_w(uint8_t data);
	void sp16_scrollx_w(uint8_t data);
	void sp16_scrolly_w(uint8_t data);
	void sp32_scrollx_w(uint8_t data);
	void sp32_scrolly_w(uint8_t data);
	void sprite_split_point_w(uint8_t data);
	void sgladiat_flipscreen_w(uint8_t data);
	void hal21_flipscreen_w(uint8_t data);
	void jcross_scroll_msb_w(uint8_t data);
	void sgladiat_scroll_msb_w(uint8_t data);
	void aso_videoattrs_w(uint8_t data);
	void tnk3_videoattrs_w(uint8_t data);
	void aso_bg_bank_w(uint8_t data);

	void register_save_state() ATTR_COLD;
	TILEMAP_MAPPER_MEMBER(marvins_tx_scan_cols);
	TILE_GET_INFO_MEMBER(marvins_get_tx_tile_info);
	TILE_GET_INFO_MEMBER(aso_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(tnk3_get_bg_tile_info);
	void tnk3_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(jcross);
	DECLARE_VIDEO_START(tnk3);
	DECLARE_VIDEO_START(sgladiat);
	DECLARE_VIDEO_START(hal21);
	DECLARE_VIDEO_START(aso);
	DECLARE_VIDEO_START(_3bpp_shadow);
	uint32_t screen_update_tnk3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_fitegolf2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(sgladiat_sndirq_update_callback);
	TIMER_CALLBACK_MEMBER(sndirq_update_callback);
	void tnk3_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, const int xscroll, const int yscroll);
	void ymirq_callback_1(int state);
	void ymirq_callback_2(int state);

	void YM3526_Y8950_sound_map(address_map &map) ATTR_COLD;
	void YM3526_YM3526_sound_map(address_map &map) ATTR_COLD;
	void YM3812_sound_map(address_map &map) ATTR_COLD;
	void aso_YM3526_sound_map(address_map &map) ATTR_COLD;
	void aso_cpuA_map(address_map &map) ATTR_COLD;
	void aso_cpuB_map(address_map &map) ATTR_COLD;
	void hal21_cpuA_map(address_map &map) ATTR_COLD;
	void hal21_cpuB_map(address_map &map) ATTR_COLD;
	void hal21_sound_map(address_map &map) ATTR_COLD;
	void jcross_cpuA_map(address_map &map) ATTR_COLD;
	void jcross_cpuB_map(address_map &map) ATTR_COLD;
	void jcross_sound_map(address_map &map) ATTR_COLD;
	void jcross_sound_portmap(address_map &map) ATTR_COLD;
	void marvins_sound_portmap(address_map &map) ATTR_COLD;
	void sgladiat_cpuA_map(address_map &map) ATTR_COLD;
	void sgladiat_cpuB_map(address_map &map) ATTR_COLD;
	void sound_common_map(address_map &map) ATTR_COLD;
	void tnk3_YM3526_sound_map(address_map &map) ATTR_COLD;
	void tnk3_cpuA_map(address_map &map) ATTR_COLD;
	void tnk3_cpuB_map(address_map &map) ATTR_COLD;
};

// with Foreground tilemap
class marvins_state : public snk_state
{
public:
	marvins_state(const machine_config &mconfig, device_type type, const char *tag) :
		snk_state(mconfig, type, tag),
		m_fg_videoram(*this, "fg_videoram")
	{ }

	void madcrush(machine_config &config) ATTR_COLD;
	void marvins(machine_config &config) ATTR_COLD;
	void vangrd2(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_shared_ptr<uint8_t> m_fg_videoram;

	tilemap_t *m_fg_tilemap = nullptr;
	int32_t m_fg_scrollx = 0;
	int32_t m_fg_scrolly = 0;

	uint8_t marvins_sound_nmi_ack_r();
	void marvins_fg_videoram_w(offs_t offset, uint8_t data);
	void marvins_palette_bank_w(uint8_t data);
	void marvins_flipscreen_w(uint8_t data);
	void fg_scrollx_w(uint8_t data);
	void fg_scrolly_w(uint8_t data);
	void marvins_scroll_msb_w(uint8_t data);

	TILE_GET_INFO_MEMBER(marvins_get_fg_tile_info);
	TILE_GET_INFO_MEMBER(marvins_get_bg_tile_info);
	void marvins_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, const int scrollx, const int scrolly, const int from, const int to);
	uint32_t screen_update_marvins(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void madcrash_cpuA_map(address_map &map) ATTR_COLD;
	void madcrash_cpuB_map(address_map &map) ATTR_COLD;
	void madcrush_cpuA_map(address_map &map) ATTR_COLD;
	void madcrush_cpuB_map(address_map &map) ATTR_COLD;
	void marvins_common_map(address_map &map) ATTR_COLD;
	void marvins_cpuA_map(address_map &map) ATTR_COLD;
	void marvins_cpuB_map(address_map &map) ATTR_COLD;
	void marvins_sound_map(address_map &map) ATTR_COLD;
};

// with trackball
class countryc_state : public snk_state
{
public:
	countryc_state(const machine_config &mconfig, device_type type, const char *tag) :
		snk_state(mconfig, type, tag),
		m_trackball_x_io(*this, "TRACKBALLX%u", 1U),
		m_trackball_y_io(*this, "TRACKBALLY%u", 1U)
	{ }

	void countryc(machine_config &config) ATTR_COLD;

	ioport_value countryc_trackball_x();
	ioport_value countryc_trackball_y();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_ioport_array<2> m_trackball_x_io;
	required_ioport_array<2> m_trackball_y_io;

	uint8_t m_countryc_trackball = 0;

	void countryc_trackball_w(uint8_t data);
	void countryc_cpuA_map(address_map &map) ATTR_COLD;
};

// with "Hard Flags" hardware
class ikari_state : public snk_state
{
public:
	ikari_state(const machine_config &mconfig, device_type type, const char *tag) :
		snk_state(mconfig, type, tag)
	{ }

	void ikari(machine_config &config) ATTR_COLD;
	void victroad(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	int32_t m_hf_posy = 0;
	int32_t m_hf_posx = 0;

	void ikari_bg_scroll_msb_w(uint8_t data);
	void ikari_sp_scroll_msb_w(uint8_t data);
	void ikari_unknown_video_w(uint8_t data);
	void hardflags_scrollx_w(uint8_t data);
	void hardflags_scrolly_w(uint8_t data);
	void hardflags_scroll_msb_w(uint8_t data);
	uint8_t hardflags7_r();

	TILE_GET_INFO_MEMBER(ikari_get_tx_tile_info);
	TILE_GET_INFO_MEMBER(ikari_get_bg_tile_info);
	uint32_t screen_update_ikari(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void ikari_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, const int start, const int xscroll, const int yscroll, const uint8_t *source, const int gfxnum );
	int hardflags_check(int num);
	template <unsigned Num> uint8_t hardflags_check8();

	void ikari_common_map(address_map &map) ATTR_COLD;
	void ikari_cpuA_map(address_map &map) ATTR_COLD;
	void ikari_cpuB_map(address_map &map) ATTR_COLD;
};

// with "Turbo Front" hardware, Higher screen resolution
class bermudat_state : public snk_state
{
public:
	bermudat_state(const machine_config &mconfig, device_type type, const char *tag) :
		snk_state(mconfig, type, tag)
	{ }

	void bermudat(machine_config &config) ATTR_COLD;
	void chopper1(machine_config &config) ATTR_COLD;
	void choppera(machine_config &config) ATTR_COLD;
	void psychos(machine_config &config) ATTR_COLD;
	void tdfever(machine_config &config) ATTR_COLD;
	void tdfever2(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

	bool m_is_psychos = false;
	uint8_t m_empty_tile[16*16]{};

	int32_t m_tc16_posy = 0;
	int32_t m_tc16_posx = 0;
	int32_t m_tc32_posy = 0;
	int32_t m_tc32_posx = 0;

	void gwar_tx_bank_w(uint8_t data);
	void gwar_videoattrs_w(uint8_t data);
	void gwara_videoattrs_w(uint8_t data);
	void gwara_sp_scroll_msb_w(uint8_t data);
	void tdfever_sp_scroll_msb_w(uint8_t data);
	void tdfever_spriteram_w(offs_t offset, uint8_t data);
	void turbocheck16_1_w(uint8_t data);
	void turbocheck16_2_w(uint8_t data);
	void turbocheck32_1_w(uint8_t data);
	void turbocheck32_2_w(uint8_t data);
	void turbocheck_msb_w(uint8_t data);
	int turbofront_check(bool small, int num);
	template <bool Small, unsigned Num> uint8_t turbofront_check8();

	TILE_GET_INFO_MEMBER(gwar_get_tx_tile_info);
	TILE_GET_INFO_MEMBER(gwar_get_bg_tile_info);
	DECLARE_VIDEO_START(gwar);
	DECLARE_VIDEO_START(psychos);
	DECLARE_VIDEO_START(_4bpp_shadow);
	DECLARE_VIDEO_START(tdfever);
	uint32_t screen_update_gwar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_tdfever(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void tdfever_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect,  const int xscroll, const int yscroll, const uint8_t *source, const int gfxnum, const bool hw_xflip, const int from, const int to);

	void bermudat_common_map(address_map &map) ATTR_COLD;
	void bermudat_cpuA_map(address_map &map) ATTR_COLD;
	void bermudat_cpuB_map(address_map &map) ATTR_COLD;
	void chopper1_sound_map(address_map &map) ATTR_COLD;
	void gwar_common_map(address_map &map) ATTR_COLD;
	void gwar_cpuA_map(address_map &map) ATTR_COLD;
	void gwar_cpuB_map(address_map &map) ATTR_COLD;
	void tdfever_common_map(address_map &map) ATTR_COLD;
	void tdfever_cpuA_map(address_map &map) ATTR_COLD;
	void tdfever_cpuB_map(address_map &map) ATTR_COLD;
	void tdfever2_sound_map(address_map &map) ATTR_COLD;
};

// bermudat_state with rotary joystick protection
class gwar_state : public bermudat_state
{
public:
	gwar_state(const machine_config &mconfig, device_type type, const char *tag) :
		bermudat_state(mconfig, type, tag),
		m_rot_io(*this, "P%uROT", 1U)
	{ }

	void gwar(machine_config &config) ATTR_COLD;
	void gwara(machine_config &config) ATTR_COLD;

	template <int Which> ioport_value gwar_rotary();

protected:
	virtual void machine_start() override ATTR_COLD;

	required_ioport_array<2> m_rot_io;

	int32_t m_last_value[2]{};
	int32_t m_cp_count[2]{};

	void gwara_common_map(address_map &map) ATTR_COLD;
	void gwara_cpuA_map(address_map &map) ATTR_COLD;
	void gwara_cpuB_map(address_map &map) ATTR_COLD;
};

// gwar_state bootleg with joystick mode
class gwarb_state : public gwar_state
{
public:
	gwarb_state(const machine_config &mconfig, device_type type, const char *tag) :
		gwar_state(mconfig, type, tag),
		m_joymode_io(*this, "JOYSTICK_MODE")
	{ }

	template <int Which> ioport_value gwarb_rotary() ATTR_COLD;

private:
	required_ioport m_joymode_io;
};

#endif // MAME_SNK_SNK_H
