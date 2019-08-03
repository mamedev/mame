// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#ifndef MAME_INCLUDES_NEMESIS_H
#define MAME_INCLUDES_NEMESIS_H

#pragma once

#include "machine/timer.h"
#include "sound/flt_rc.h"
#include "sound/k007232.h"
#include "sound/k005289.h"
#include "sound/vlm5030.h"

#include "emupal.h"
#include "screen.h"


class nemesis_state : public driver_device
{
public:
	nemesis_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_charram(*this, "charram"),
		m_xscroll1(*this, "xscroll1"),
		m_xscroll2(*this, "xscroll2"),
		m_yscroll2(*this, "yscroll2"),
		m_yscroll1(*this, "yscroll1"),
		m_videoram1(*this, "videoram1"),
		m_videoram2(*this, "videoram2"),
		m_colorram1(*this, "colorram1"),
		m_colorram2(*this, "colorram2"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_gx400_shared_ram(*this, "gx400_shared"),
		m_bubsys_shared_ram(*this, "bubsys_shared"),
		m_bubsys_control_ram(*this, "bubsys_control"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_filter1(*this, "filter1"),
		m_filter2(*this, "filter2"),
		m_filter3(*this, "filter3"),
		m_filter4(*this, "filter4"),
		m_k007232(*this, "k007232"),
		m_k005289(*this, "k005289"),
		m_vlm(*this, "vlm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	void nyanpani(machine_config &config);
	void konamigt(machine_config &config);
	void rf2_gx400(machine_config &config);
	void gx400(machine_config &config);
	void bubsys(machine_config &config);
	void hcrash(machine_config &config);
	void salamand(machine_config &config);
	void citybomb(machine_config &config);
	void nemesis(machine_config &config);
	void blkpnthr(machine_config &config);

	void bubsys_init();

private:
	/* memory pointers */
	required_shared_ptr<uint16_t> m_charram;
	required_shared_ptr<uint16_t> m_xscroll1;
	required_shared_ptr<uint16_t> m_xscroll2;
	required_shared_ptr<uint16_t> m_yscroll2;
	required_shared_ptr<uint16_t> m_yscroll1;
	required_shared_ptr<uint16_t> m_videoram1;
	required_shared_ptr<uint16_t> m_videoram2;
	required_shared_ptr<uint16_t> m_colorram1;
	required_shared_ptr<uint16_t> m_colorram2;
	required_shared_ptr<uint16_t> m_spriteram;
	optional_shared_ptr<uint16_t> m_paletteram;
	optional_shared_ptr<uint8_t> m_gx400_shared_ram;
	optional_shared_ptr<uint16_t> m_bubsys_shared_ram;
	optional_shared_ptr<uint16_t> m_bubsys_control_ram;

	/* video-related */
	tilemap_t *m_background;
	tilemap_t *m_foreground;
	int       m_spriteram_words;
	int       m_tilemap_flip;
	int       m_flipscreen;
	uint8_t     m_irq_port_last;
	uint8_t     m_blank_tile[8*8];
	uint8_t     m_palette_lookup[32];

	/* misc */
	int       m_irq_on;
	int       m_irq1_on;
	int       m_irq2_on;
	int       m_irq4_on;
	uint16_t    m_selected_ip; /* Copied from WEC Le Mans 24 driver, explicity needed for Hyper Crash */
	int       m_gx400_irq1_cnt;
	uint8_t     m_frame_counter;
	uint16_t    m_scanline_counter;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<filter_rc_device> m_filter1;
	optional_device<filter_rc_device> m_filter2;
	optional_device<filter_rc_device> m_filter3;
	optional_device<filter_rc_device> m_filter4;
	optional_device<k007232_device> m_k007232;
	optional_device<k005289_device> m_k005289;
	optional_device<vlm5030_device> m_vlm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	DECLARE_WRITE_LINE_MEMBER(irq_enable_w);
	DECLARE_WRITE_LINE_MEMBER(irq1_enable_w);
	DECLARE_WRITE_LINE_MEMBER(irq2_enable_w);
	DECLARE_WRITE_LINE_MEMBER(irq4_enable_w);
	DECLARE_WRITE_LINE_MEMBER(coin1_lockout_w);
	DECLARE_WRITE_LINE_MEMBER(coin2_lockout_w);
	DECLARE_WRITE_LINE_MEMBER(sound_irq_w);
	DECLARE_WRITE_LINE_MEMBER(sound_nmi_w);
	DECLARE_READ16_MEMBER(gx400_sharedram_word_r);
	DECLARE_WRITE16_MEMBER(gx400_sharedram_word_w);
	DECLARE_READ16_MEMBER(konamigt_input_word_r);
	DECLARE_WRITE16_MEMBER(selected_ip_word_w);
	DECLARE_READ16_MEMBER(selected_ip_word_r);
	DECLARE_WRITE16_MEMBER(bubsys_mcu_w);
	DECLARE_READ8_MEMBER(wd_r);
	DECLARE_WRITE_LINE_MEMBER(gfx_flipx_w);
	DECLARE_WRITE_LINE_MEMBER(gfx_flipy_w);
	DECLARE_WRITE16_MEMBER(salamand_control_port_word_w);
	DECLARE_WRITE16_MEMBER(nemesis_palette_word_w);
	DECLARE_WRITE16_MEMBER(nemesis_videoram1_word_w);
	DECLARE_WRITE16_MEMBER(nemesis_videoram2_word_w);
	DECLARE_WRITE16_MEMBER(nemesis_colorram1_word_w);
	DECLARE_WRITE16_MEMBER(nemesis_colorram2_word_w);
	DECLARE_WRITE16_MEMBER(nemesis_charram_word_w);
	DECLARE_WRITE8_MEMBER(nemesis_filter_w);
	DECLARE_WRITE8_MEMBER(gx400_speech_start_w);
	DECLARE_WRITE8_MEMBER(salamand_speech_start_w);
	DECLARE_READ8_MEMBER(nemesis_portA_r);
	DECLARE_WRITE8_MEMBER(city_sound_bank_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_nemesis(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(nemesis_vblank_irq);
	DECLARE_WRITE_LINE_MEMBER(bubsys_vblank_irq);

	DECLARE_WRITE_LINE_MEMBER(blkpnthr_vblank_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(bubsys_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(konamigt_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(hcrash_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(gx400_interrupt);
	void create_palette_lookups();
	void nemesis_postload();
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	DECLARE_WRITE8_MEMBER(volume_callback);
	void set_screen_raw_params(machine_config &config);

	void blkpnthr_map(address_map &map);
	void blkpnthr_sound_map(address_map &map);
	void city_sound_map(address_map &map);
	void citybomb_map(address_map &map);
	void gx400_map(address_map &map);
	void gx400_sound_map(address_map &map);
	void gx400_vlm_map(address_map &map);
	void hcrash_map(address_map &map);
	void konamigt_map(address_map &map);
	void nemesis_map(address_map &map);
	void nyanpani_map(address_map &map);
	void rf2_gx400_map(address_map &map);
	void sal_sound_map(address_map &map);
	void salamand_map(address_map &map);
	void salamand_vlm_map(address_map &map);
	void sound_map(address_map &map);
	void bubsys_map(address_map &map);
};

#endif // MAME_INCLUDES_NEMESIS_H
