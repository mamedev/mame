// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#ifndef MAME_INCLUDES_DEC8_H
#define MAME_INCLUDES_DEC8_H

#pragma once

#include "cpu/mcs51/mcs51.h"
#include "machine/gen_latch.h"
#include "machine/input_merger.h"
#include "sound/msm5205.h"
#include "video/bufsprite.h"
#include "video/decbac06.h"
#include "video/deckarn.h"
#include "video/decmxc06.h"
#include "video/decrmc3.h"
#include "screen.h"
#include "tilemap.h"

class dec8_state : public driver_device
{
public:
	enum
	{
		TIMER_DEC8_I8751,
		TIMER_DEC8_M6502
	};

	dec8_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_nmigate(*this, "nmigate"),
		m_spriteram(*this, "spriteram") ,
		m_msm(*this, "msm"),
		m_tilegen(*this, "tilegen%u", 1),
		m_spritegen_krn(*this, "spritegen_krn"),
		m_spritegen_mxc(*this, "spritegen_mxc"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_videoram(*this, "videoram"),
		m_bg_data(*this, "bg_data"),
		m_mainbank(*this, "mainbank"),
		m_soundbank(*this, "soundbank"),
		m_coin_port(*this, "I8751")
	{ }

	void shackled(machine_config &config);
	void meikyuh(machine_config &config);
	void lastmisn(machine_config &config);
	void csilver(machine_config &config);
	void cobracom(machine_config &config);
	void garyoret(machine_config &config);
	void srdarwin(machine_config &config);
	void ghostb(machine_config &config);
	void oscar(machine_config &config);
	void gondo(machine_config &config);

	void init_dec8();
	void init_csilver();

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<i8751_device> m_mcu;
	optional_device<input_merger_device> m_nmigate;
	required_device<buffered_spriteram8_device> m_spriteram;
	optional_device<msm5205_device> m_msm;
	optional_device_array<deco_bac06_device, 2> m_tilegen;
	optional_device<deco_karnovsprites_device> m_spritegen_krn;
	optional_device<deco_mxc06_device> m_spritegen_mxc;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<deco_rmc3_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_bg_data;

	/* memory regions */
	required_memory_bank m_mainbank;
	optional_memory_bank m_soundbank;

	uint8_t *  m_pf1_data;
	uint8_t *  m_row;
	std::unique_ptr<uint16_t[]>   m_buffered_spriteram16; // for the mxc06 sprite chip emulation (oscar, cobra)

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_pf1_tilemap;
	tilemap_t  *m_fix_tilemap;
	//int      m_scroll1[4];
	int      m_scroll2[4];
	int      m_bg_control[0x20];
	int      m_pf1_control[0x20];
	int      m_game_uses_priority;

	/* misc */
	bool     m_secclr;
	bool     m_nmi_enable;
	uint8_t  m_i8751_p2;
	int      m_i8751_port0;
	int      m_i8751_port1;
	int      m_i8751_return;
	int      m_i8751_value;
	int      m_coinage_id;
	int      m_coin1;
	int      m_coin2;
	int      m_need1;
	int      m_need2;
	int      m_cred1;
	int      m_cred2;
	int      m_credits;
	int      m_latch;
	bool     m_coin_state;
	int      m_snd;
	int      m_msm5205next;
	int      m_toggle;

	emu_timer *m_i8751_timer;
	emu_timer *m_m6502_timer;

	DECLARE_WRITE8_MEMBER(dec8_mxc06_karn_buffer_spriteram_w);
	DECLARE_READ8_MEMBER(i8751_h_r);
	DECLARE_READ8_MEMBER(i8751_l_r);
	DECLARE_WRITE8_MEMBER(i8751_reset_w);
	DECLARE_READ8_MEMBER(gondo_player_1_r);
	DECLARE_READ8_MEMBER(gondo_player_2_r);
	DECLARE_WRITE8_MEMBER(dec8_i8751_w);
	DECLARE_WRITE8_MEMBER(csilver_i8751_w);
	DECLARE_WRITE8_MEMBER(dec8_bank_w);
	DECLARE_WRITE8_MEMBER(ghostb_bank_w);
	DECLARE_WRITE8_MEMBER(csilver_control_w);
	DECLARE_WRITE8_MEMBER(dec8_sound_w);
	DECLARE_WRITE8_MEMBER(csilver_adpcm_data_w);
	DECLARE_WRITE8_MEMBER(csilver_sound_bank_w);
	DECLARE_WRITE8_MEMBER(main_irq_on_w);
	DECLARE_WRITE8_MEMBER(main_irq_off_w);
	DECLARE_WRITE8_MEMBER(main_firq_off_w);
	DECLARE_WRITE8_MEMBER(sub_irq_on_w);
	DECLARE_WRITE8_MEMBER(sub_irq_off_w);
	DECLARE_WRITE8_MEMBER(sub_firq_off_w);
	DECLARE_WRITE8_MEMBER(flip_screen_w);
	DECLARE_READ8_MEMBER(i8751_port0_r);
	DECLARE_WRITE8_MEMBER(i8751_port0_w);
	DECLARE_READ8_MEMBER(i8751_port1_r);
	DECLARE_WRITE8_MEMBER(i8751_port1_w);
	DECLARE_WRITE8_MEMBER(gondo_mcu_to_main_w);
	DECLARE_WRITE8_MEMBER(shackled_mcu_to_main_w);
	DECLARE_WRITE8_MEMBER(srdarwin_mcu_to_main_w);
	DECLARE_WRITE8_MEMBER(dec8_bg_data_w);
	DECLARE_READ8_MEMBER(dec8_bg_data_r);
	DECLARE_WRITE8_MEMBER(dec8_videoram_w);
	DECLARE_WRITE8_MEMBER(srdarwin_videoram_w);
	DECLARE_WRITE8_MEMBER(dec8_scroll2_w);
	DECLARE_WRITE8_MEMBER(srdarwin_control_w);
	DECLARE_WRITE8_MEMBER(lastmisn_control_w);
	DECLARE_WRITE8_MEMBER(shackled_control_w);
	DECLARE_WRITE8_MEMBER(lastmisn_scrollx_w);
	DECLARE_WRITE8_MEMBER(lastmisn_scrolly_w);
	DECLARE_WRITE8_MEMBER(gondo_scroll_w);
	DECLARE_READ8_MEMBER(csilver_adpcm_reset_r);
	TILE_GET_INFO_MEMBER(get_cobracom_fix_tile_info);
	TILE_GET_INFO_MEMBER(get_ghostb_fix_tile_info);
	TILE_GET_INFO_MEMBER(get_oscar_fix_tile_info);
	TILEMAP_MAPPER_MEMBER(lastmisn_scan_rows);
	TILE_GET_INFO_MEMBER(get_lastmisn_tile_info);
	TILE_GET_INFO_MEMBER(get_lastmisn_fix_tile_info);
	TILE_GET_INFO_MEMBER(get_srdarwin_fix_tile_info);
	TILE_GET_INFO_MEMBER(get_srdarwin_tile_info);
	TILE_GET_INFO_MEMBER(get_gondo_fix_tile_info);
	TILE_GET_INFO_MEMBER(get_gondo_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_VIDEO_START(lastmisn);
	DECLARE_VIDEO_START(shackled);
	DECLARE_VIDEO_START(gondo);
	DECLARE_VIDEO_START(garyoret);
	DECLARE_VIDEO_START(ghostb);
	DECLARE_VIDEO_START(oscar);
	DECLARE_VIDEO_START(srdarwin);
	DECLARE_VIDEO_START(cobracom);
	void allocate_buffered_spriteram16();
	uint32_t screen_update_lastmisn(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_shackled(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_gondo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_garyoret(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_ghostb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_oscar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_srdarwin(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_cobracom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_dec8);
	DECLARE_WRITE_LINE_MEMBER(oscar_coin_irq);
	DECLARE_WRITE8_MEMBER(oscar_coin_clear_w);
	DECLARE_WRITE_LINE_MEMBER(shackled_coin_irq);
	void srdarwin_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &primap);
	void gondo_colpri_cb(u32 &colour, u32 &pri_mask);
	void cobracom_colpri_cb(u32 &colour, u32 &pri_mask);
	DECLARE_WRITE_LINE_MEMBER(csilver_adpcm_int);

	void set_screen_raw_params_data_east(machine_config &config);

	void cobra_map(address_map &map);
	void csilver_map(address_map &map);
	void csilver_s_map(address_map &map);
	void csilver_sub_map(address_map &map);
	void dec8_s_map(address_map &map);
	void garyoret_map(address_map &map);
	void gondo_map(address_map &map);
	void lastmisn_map(address_map &map);
	void lastmisn_sub_map(address_map &map);
	void meikyuh_map(address_map &map);
	void oscar_map(address_map &map);
	void oscar_s_map(address_map &map);
	void oscar_sub_map(address_map &map);
	void shackled_map(address_map &map);
	void shackled_sub_map(address_map &map);
	void srdarwin_map(address_map &map);
	void ym3526_s_map(address_map &map);
protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	/* ports */
	optional_ioport m_coin_port;
};

#endif // MAME_INCLUDES_DEC8_H
