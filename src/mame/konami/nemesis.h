// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#ifndef MAME_KONAMI_NEMESIS_H
#define MAME_KONAMI_NEMESIS_H

#pragma once

#include "machine/timer.h"
#include "sound/flt_rc.h"
#include "sound/k007232.h"
#include "sound/k005289.h"
#include "sound/vlm5030.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


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
	void bubsys_twinbeeb_init();

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
	tilemap_t *m_background = nullptr;
	tilemap_t *m_foreground = nullptr;
	int       m_spriteram_words = 0;
	int       m_tilemap_flip = 0;
	int       m_flipscreen = 0;
	uint8_t   m_irq_port_last = 0;
	uint8_t   m_blank_tile[8*8] = { };
	uint8_t   m_palette_lookup[32] = { };

	/* misc */
	int       m_irq_on = 0;
	int       m_irq1_on = 0;
	int       m_irq2_on = 0;
	int       m_irq4_on = 0;
	uint8_t   m_selected_ip = 0; // needed for Hyper Crash
	int       m_gx400_irq1_cnt = 0;
	uint8_t   m_gx400_speech_offset = 0;
	uint16_t  m_scanline_counter = 0;

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

	void irq_enable_w(int state);
	void irq1_enable_w(int state);
	void irq2_enable_w(int state);
	void irq4_enable_w(int state);
	void coin1_lockout_w(int state);
	void coin2_lockout_w(int state);
	void sound_irq_w(int state);
	void sound_nmi_w(int state);
	uint16_t gx400_sharedram_word_r(offs_t offset);
	void gx400_sharedram_word_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t konamigt_input_word_r();
	void selected_ip_w(uint8_t data);
	uint8_t selected_ip_r();
	void bubsys_mcu_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void gfx_flipx_w(int state);
	void gfx_flipy_w(int state);
	void salamand_control_port_word_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void nemesis_palette_word_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void nemesis_videoram1_word_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void nemesis_videoram2_word_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void nemesis_colorram1_word_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void nemesis_colorram2_word_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void nemesis_charram_word_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void nemesis_filter_w(offs_t offset, uint8_t data);
	void gx400_speech_w(offs_t offset, uint8_t data);
	void salamand_speech_start_w(uint8_t data);
	uint8_t salamand_speech_busy_r();
	uint8_t nemesis_portA_r();
	void city_sound_bank_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update_nemesis(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void nemesis_vblank_irq(int state);
	void bubsys_vblank_irq(int state);

	void blkpnthr_vblank_irq(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(bubsys_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(konamigt_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(hcrash_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(gx400_interrupt);
	void create_palette_lookups();
	void nemesis_postload();
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void volume_callback(uint8_t data);
	void set_screen_raw_params(machine_config &config);

	void blkpnthr_map(address_map &map) ATTR_COLD;
	void blkpnthr_sound_map(address_map &map) ATTR_COLD;
	void city_sound_map(address_map &map) ATTR_COLD;
	void citybomb_map(address_map &map) ATTR_COLD;
	void gx400_map(address_map &map) ATTR_COLD;
	void gx400_sound_map(address_map &map) ATTR_COLD;
	void gx400_vlm_map(address_map &map) ATTR_COLD;
	void hcrash_map(address_map &map) ATTR_COLD;
	void konamigt_map(address_map &map) ATTR_COLD;
	void nemesis_map(address_map &map) ATTR_COLD;
	void nyanpani_map(address_map &map) ATTR_COLD;
	void rf2_gx400_map(address_map &map) ATTR_COLD;
	void sal_sound_map(address_map &map) ATTR_COLD;
	void salamand_map(address_map &map) ATTR_COLD;
	void salamand_vlm_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void bubsys_map(address_map &map) ATTR_COLD;
};

#endif // MAME_KONAMI_NEMESIS_H
