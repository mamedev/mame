// license:BSD-3-Clause
// copyright-holders:Dan Boris, Aaron Giles
/*************************************************************************

    Atari Return of the Jedi hardware

*************************************************************************/
#ifndef MAME_INCLUDES_JEDI_H
#define MAME_INCLUDES_JEDI_H

#pragma once

#include "machine/gen_latch.h"
#include "machine/x2212.h"
#include "sound/tms5220.h"
#include "emupal.h"
#include "screen.h"

#define DEBUG_GFXDECODE 0 // GFX layout for debug

/* oscillators and clocks */
#define JEDI_MAIN_CPU_OSC       (XTAL(10'000'000))
#define JEDI_AUDIO_CPU_OSC      (XTAL(12'096'000))
#define JEDI_MAIN_CPU_CLOCK     (JEDI_MAIN_CPU_OSC / 4)
#define JEDI_AUDIO_CPU_CLOCK    (JEDI_AUDIO_CPU_OSC / 8)
#define JEDI_POKEY_CLOCK        (JEDI_AUDIO_CPU_CLOCK)
#define JEDI_TMS5220_CLOCK      (JEDI_AUDIO_CPU_OSC / 2 / 9) /* div by 9 is via a binary counter that counts from 7 to 16 */


class jedi_state : public driver_device
{
public:
	jedi_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_backgroundram(*this, "backgroundram"),
		m_foregroundram(*this, "foregroundram"),
		m_spriteram(*this, "spriteram"),
		m_smoothing_table(*this, "smoothing_table"),
		m_tx_gfx(*this, "tx_gfx"),
		m_bg_gfx(*this, "bg_gfx"),
		m_spr_gfx(*this, "spr_gfx"),
		m_proms(*this, "proms"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_soundlatch(*this, "soundlatch"),
		m_sacklatch(*this, "sacklatch"),
		m_tms(*this, "tms"),
		m_novram(*this, "novram12%c", 'b'),
#if DEBUG_GFXDECODE
		m_gfxdecode(*this, "gfxdecode"),
#endif
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_mainbank(*this, "mainbank")
	{ }

	DECLARE_CUSTOM_INPUT_MEMBER(jedi_audio_comm_stat_r);
	void jedi(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	void main_irq_ack_w(u8 data);
	void rom_banksel_w(u8 data);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_left_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_right_w);
	u8 novram_data_r(address_space &space, offs_t offset);
	void novram_data_w(offs_t offset, u8 data);
	void novram_recall_w(offs_t offset, u8 data);
	void novram_store_w(u8 data);
	void vscroll_w(offs_t offset, u8 data);
	void hscroll_w(offs_t offset, u8 data);
	void irq_ack_w(u8 data);
	DECLARE_WRITE_LINE_MEMBER(audio_reset_w);
	u8 audio_comm_stat_r();
	void speech_strobe_w(offs_t offset, u8 data);
	u8 speech_ready_r();
	void speech_reset_w(u8 data);
	DECLARE_WRITE_LINE_MEMBER(foreground_bank_w);
	DECLARE_WRITE_LINE_MEMBER(video_off_w);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(generate_interrupt);
	static rgb_t jedi_IRGB_3333(u32 raw);
	void do_pen_lookup(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_background_and_text(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void jedi_audio(machine_config &config);
	void jedi_video(machine_config &config);
	void audio_map(address_map &map);
	void main_map(address_map &map);

	/* machine state */
	emu_timer *m_interrupt_timer = nullptr;

	/* video state */
	required_shared_ptr<u8> m_backgroundram;
	required_shared_ptr<u8> m_foregroundram;
	required_shared_ptr<u8> m_spriteram;
	required_shared_ptr<u8> m_smoothing_table;
	required_region_ptr<u8> m_tx_gfx;
	required_region_ptr<u8> m_bg_gfx;
	required_region_ptr<u8> m_spr_gfx;
	required_region_ptr<u8> m_proms;
	u32 m_vscroll = 0;
	u32 m_hscroll = 0;
	bool m_foreground_bank = false;
	bool m_video_off = false;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_sacklatch;
	required_device<tms5220_device> m_tms;
	required_device_array<x2212_device, 2> m_novram;
#if DEBUG_GFXDECODE
	required_device<gfxdecode_device> m_gfxdecode;
	std::unique_ptr<u8[]> m_gfxdata;
#endif
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_memory_bank m_mainbank;
};

#endif // MAME_INCLUDES_JEDI_H
