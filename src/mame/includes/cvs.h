// license:BSD-3-Clause
// copyright-holders:Mike Coates, Couriersud
/***************************************************************************

    Century CVS System

****************************************************************************/
#ifndef MAME_INCLUDES_CVS_H
#define MAME_INCLUDES_CVS_H

#pragma once

#include "cpu/s2650/s2650.h"
#include "machine/gen_latch.h"
#include "machine/s2636.h"
#include "sound/dac.h"
#include "sound/tms5110.h"
#include "emupal.h"
#include "screen.h"

#define CVS_S2636_Y_OFFSET     (-5)
#define CVS_S2636_X_OFFSET     (-26)
#define CVS_MAX_STARS          250

struct cvs_star
{
	int x, y, code;
};

class cvs_state : public driver_device
{
public:
	cvs_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_video_ram(*this, "video_ram")
		, m_bullet_ram(*this, "bullet_ram")
		, m_cvs_4_bit_dac_data(*this, "4bit_dac")
		, m_tms5110_ctl_data(*this, "tms5110_ctl")
		, m_dac3_state(*this, "dac3_state")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_speechcpu(*this, "speechcpu")
		, m_dac2(*this, "dac2")
		, m_dac3(*this, "dac3")
		, m_tms5110(*this, "tms")
		, m_s2636(*this, "s2636%u", 0U)
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
		, m_lamps(*this, "lamp%u", 1U)
	{ }

	void init_raiders();
	void init_huncholy();
	void init_hero();
	void init_superbik();
	void init_hunchbaka();
	void cvs(machine_config &config);

protected:

	DECLARE_WRITE_LINE_MEMBER(write_s2650_flag); // used by galaxia_state
	DECLARE_READ8_MEMBER(huncholy_prot_r);
	DECLARE_READ8_MEMBER(superbik_prot_r);
	DECLARE_READ8_MEMBER(hero_prot_r);
	DECLARE_READ_LINE_MEMBER(speech_rom_read_bit);
	DECLARE_WRITE_LINE_MEMBER(cvs_slave_cpu_interrupt);
	DECLARE_READ8_MEMBER(cvs_input_r);
	DECLARE_WRITE8_MEMBER(cvs_speech_rom_address_lo_w);
	DECLARE_WRITE8_MEMBER(cvs_speech_rom_address_hi_w);
	DECLARE_READ8_MEMBER(cvs_speech_command_r);
	DECLARE_WRITE8_MEMBER(audio_command_w);
	DECLARE_READ8_MEMBER(cvs_video_or_color_ram_r);
	DECLARE_WRITE8_MEMBER(cvs_video_or_color_ram_w);
	DECLARE_READ8_MEMBER(cvs_bullet_ram_or_palette_r);
	DECLARE_WRITE8_MEMBER(cvs_bullet_ram_or_palette_w);
	DECLARE_READ8_MEMBER(cvs_s2636_0_or_character_ram_r);
	DECLARE_WRITE8_MEMBER(cvs_s2636_0_or_character_ram_w);
	DECLARE_READ8_MEMBER(cvs_s2636_1_or_character_ram_r);
	DECLARE_WRITE8_MEMBER(cvs_s2636_1_or_character_ram_w);
	DECLARE_READ8_MEMBER(cvs_s2636_2_or_character_ram_r);
	DECLARE_WRITE8_MEMBER(cvs_s2636_2_or_character_ram_w);
	DECLARE_WRITE8_MEMBER(cvs_video_fx_w);
	DECLARE_READ8_MEMBER(cvs_collision_r);
	DECLARE_READ8_MEMBER(cvs_collision_clear);
	DECLARE_WRITE8_MEMBER(cvs_scroll_w);
	DECLARE_READ_LINE_MEMBER(tms_clock_r);
	DECLARE_WRITE8_MEMBER(cvs_4_bit_dac_data_w);
	DECLARE_WRITE8_MEMBER(cvs_unknown_w);
	DECLARE_WRITE8_MEMBER(cvs_tms5110_ctl_w);
	DECLARE_WRITE8_MEMBER(cvs_tms5110_pdc_w);
	DECLARE_VIDEO_START(cvs);
	void cvs_palette(palette_device &palette) const;
	uint32_t screen_update_cvs(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(cvs_main_cpu_interrupt);
	TIMER_CALLBACK_MEMBER(cvs_393hz_timer_cb);
	void set_pens();
	void cvs_scroll_stars();
	void cvs_init_stars();
	void cvs_update_stars(bitmap_ind16 &bitmap, const rectangle &cliprect, const pen_t star_pen, bool update_always);
	void start_393hz_timer();
	void cvs_dac_cpu_map(address_map &map);
	void cvs_main_cpu_data_map(address_map &map);
	void cvs_main_cpu_io_map(address_map &map);
	void cvs_main_cpu_map(address_map &map);
	void cvs_speech_cpu_map(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_video_ram;
	required_shared_ptr<uint8_t> m_bullet_ram;
	optional_shared_ptr<uint8_t> m_cvs_4_bit_dac_data;
	optional_shared_ptr<uint8_t> m_tms5110_ctl_data;
	optional_shared_ptr<uint8_t> m_dac3_state;

	/* video-related */
	struct cvs_star m_stars[CVS_MAX_STARS];
	bitmap_ind16   m_collision_background;
	bitmap_ind16   m_background_bitmap;
	bitmap_ind16   m_scrolled_collision_background;
	int        m_collision_register;
	int        m_total_stars;
	int        m_stars_on;
	uint8_t      m_scroll_reg;
	int        m_stars_scroll;

	/* misc */
	int m_s2650_flag;
	emu_timer  *m_cvs_393hz_timer;
	uint8_t      m_cvs_393hz_clock;
	uint8_t      m_protection_counter;

	uint8_t      m_character_banking_mode;
	uint16_t     m_character_ram_page_start;
	uint16_t     m_speech_rom_bit_address;

	/* devices */
	required_device<s2650_device> m_maincpu;
	optional_device<s2650_device> m_audiocpu;
	optional_device<s2650_device> m_speechcpu;
	optional_device<dac_byte_interface> m_dac2;
	optional_device<dac_bit_interface> m_dac3;
	optional_device<tms5110_device> m_tms5110;
	optional_device_array<s2636_device, 3> m_s2636;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch;
	output_finder<2> m_lamps;

	/* memory */
	uint8_t      m_color_ram[0x400];
	uint8_t      m_palette_ram[0x10];
	uint8_t      m_character_ram[3 * 0x800];  /* only half is used, but
	                                           by allocating twice the amount,
	                                           we can use the same gfx_layout */
};

#endif // MAME_INCLUDES_CVS_H
