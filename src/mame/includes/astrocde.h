// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Mike Coates, Frank Palazzolo, Aaron Giles
/***************************************************************************

    Bally Astrocade-based hardware

***************************************************************************/
#ifndef MAME_INCLUDES_ASTROCDE_H
#define MAME_INCLUDES_ASTROCDE_H

#pragma once

#include "cpu/z80/z80.h"
#include "machine/bankdev.h"
#include "machine/gen_latch.h"
#include "sound/astrocde.h"
#include "sound/samples.h"
#include "sound/votrax.h"
#include "emupal.h"
#include "screen.h"

#define ASTROCADE_CLOCK     (XTAL(14'318'181)/2)

#define AC_SOUND_PRESENT    (0x01)
#define AC_LIGHTPEN_INTS    (0x02)
#define AC_STARS            (0x04)
#define AC_MONITOR_BW       (0x08)


class astrocde_state : public driver_device
{
public:
	enum
	{
		TIMER_INTERRUPT_OFF,
		TIMER_SCANLINE
	};

	astrocde_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_votrax(*this, "votrax"),
		m_astrocade_sound1(*this, "astrocade1"),
		m_videoram(*this, "videoram"),
		m_protected_ram(*this, "protected_ram"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_bank4000(*this, "bank4000"),
		m_bank8000(*this, "bank8000"),
		m_handle(*this, "P%uHANDLE", 1U),
		m_interrupt_scanline(0xff)
	{ }

	required_device<cpu_device> m_maincpu;
	optional_device<votrax_sc01_device> m_votrax;
	optional_device<astrocade_io_device> m_astrocade_sound1;
	optional_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_protected_ram;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch;
	optional_device<address_map_bank_device> m_bank4000;
	optional_memory_bank m_bank8000;
	optional_ioport_array<4> m_handle;

	uint8_t m_video_config;
	uint8_t m_sparkle[4];
	char m_totalword[256];
	char *m_totalword_ptr;
	char m_oldword[256];
	int m_plural;
	uint8_t m_ram_write_enable;
	uint8_t m_input_select;
	std::unique_ptr<uint8_t[]> m_sparklestar;
	uint8_t m_interrupt_enabl;
	uint8_t m_interrupt_vector;
	uint8_t m_interrupt_scanline;
	uint8_t m_vertical_feedback;
	uint8_t m_horizontal_feedback;
	emu_timer *m_scanline_timer;
	emu_timer *m_intoff_timer;
	uint8_t m_colors[8];
	uint8_t m_colorsplit;
	uint8_t m_bgdata;
	uint8_t m_vblank;
	uint8_t m_video_mode;
	uint8_t m_funcgen_expand_color[2];
	uint8_t m_funcgen_control;
	uint8_t m_funcgen_expand_count;
	uint8_t m_funcgen_rotate_count;
	uint8_t m_funcgen_rotate_data[4];
	uint8_t m_funcgen_shift_prev_data;
	uint8_t m_funcgen_intercept;
	uint16_t m_pattern_source;
	uint8_t m_pattern_mode;
	uint16_t m_pattern_dest;
	uint8_t m_pattern_skip;
	uint8_t m_pattern_width;
	uint8_t m_pattern_height;
	std::unique_ptr<uint16_t[]> m_profpac_videoram;
	uint16_t m_profpac_palette[16];
	uint8_t m_profpac_colormap[4];
	uint8_t m_profpac_intercept;
	uint8_t m_profpac_vispage;
	uint8_t m_profpac_readpage;
	uint8_t m_profpac_readshift;
	uint8_t m_profpac_writepage;
	uint8_t m_profpac_writemode;
	uint16_t m_profpac_writemask;
	uint8_t m_profpac_vw;
	DECLARE_WRITE8_MEMBER(protected_ram_enable_w);
	DECLARE_READ8_MEMBER(protected_ram_r);
	DECLARE_WRITE8_MEMBER(protected_ram_w);
	DECLARE_READ8_MEMBER(input_mux_r);
	template<int Coin> DECLARE_WRITE_LINE_MEMBER(coin_counter_w);
	template<int Bit> DECLARE_WRITE_LINE_MEMBER(sparkle_w);
	DECLARE_WRITE_LINE_MEMBER(gorf_sound_switch_w);
	DECLARE_WRITE8_MEMBER(profpac_banksw_w);
	DECLARE_WRITE8_MEMBER(demndrgn_banksw_w);
	DECLARE_READ8_MEMBER(video_register_r);
	DECLARE_WRITE8_MEMBER(video_register_w);
	DECLARE_WRITE8_MEMBER(astrocade_funcgen_w);
	DECLARE_WRITE8_MEMBER(expand_register_w);
	DECLARE_WRITE8_MEMBER(astrocade_pattern_board_w);
	DECLARE_WRITE8_MEMBER(profpac_page_select_w);
	DECLARE_READ8_MEMBER(profpac_intercept_r);
	DECLARE_WRITE8_MEMBER(profpac_screenram_ctrl_w);
	DECLARE_READ8_MEMBER(profpac_videoram_r);
	DECLARE_WRITE8_MEMBER(profpac_videoram_w);
	DECLARE_INPUT_CHANGED_MEMBER(spacezap_monitor);
	void init_profpac();
	void init_spacezap();
	void init_robby();
	void init_wow();
	void init_tenpindx();
	void init_seawolf2();
	void init_demndrgn();
	void init_ebases();
	void init_gorf();
	void init_astrocde();
	virtual void video_start() override;
	void astrocade_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(profpac);
	void profpac_palette(palette_device &palette) const;
	uint32_t screen_update_astrocde(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_profpac(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(scanline_callback);
	inline int mame_vpos_to_astrocade_vpos(int scanline);
	void init_savestate();
	void astrocade_trigger_lightpen(uint8_t vfeedback, uint8_t hfeedback);
	inline void increment_source(uint8_t curwidth, uint8_t *u13ff);
	inline void increment_dest(uint8_t curwidth);
	void execute_blit();
	void init_sparklestar();
	virtual void machine_start() override;

	DECLARE_WRITE8_MEMBER(votrax_speech_w);
	CUSTOM_INPUT_MEMBER( votrax_speech_status_r );

	void astrocade_base(machine_config &config);
	void astrocade_16color_base(machine_config &config);
	void astrocade_mono_sound(machine_config &config);
	void astrocade_stereo_sound(machine_config &config);
	void spacezap(machine_config &config);
	void gorf(machine_config &config);
	void profpac(machine_config &config);
	void robby(machine_config &config);
	void wow(machine_config &config);
	void bank4000_map(address_map &map);
	void demndrgn_map(address_map &map);
	void port_map(address_map &map);
	void port_map_16col_pattern(address_map &map);
	void port_map_16col_pattern_nosound(address_map &map);
	void port_map_mono_pattern(address_map &map);
	void port_map_stereo_pattern(address_map &map);
	void profpac_bank4000_map(address_map &map);
	void profpac_map(address_map &map);
	void robby_map(address_map &map);
	void seawolf2_map(address_map &map);
	void spacezap_map(address_map &map);
	void wow_map(address_map &map);
protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

class seawolf2_state : public astrocde_state
{
public:
	seawolf2_state(const machine_config &mconfig, device_type type, const char *tag) :
		astrocde_state(mconfig, type, tag),
		m_samples(*this, "samples")
	{ }

	void seawolf2(machine_config &config);
private:
	DECLARE_WRITE8_MEMBER(sound_1_w);
	DECLARE_WRITE8_MEMBER(sound_2_w);

	void port_map_discrete(address_map &map);

	virtual void machine_start() override;

	required_device<samples_device> m_samples;
	uint8_t m_port_1_last;
	uint8_t m_port_2_last;
};

class ebases_state : public astrocde_state
{
public:
	ebases_state(const machine_config &mconfig, device_type type, const char *tag) :
		astrocde_state(mconfig, type, tag),
		m_trackball(*this, {"TRACKX2", "TRACKY2", "TRACKX1", "TRACKY1"})
	{ }

	void ebases(machine_config &config);
	DECLARE_CUSTOM_INPUT_MEMBER(trackball_r);
private:
	DECLARE_WRITE8_MEMBER(trackball_select_w);
	DECLARE_WRITE8_MEMBER(coin_w);

	void ebases_map(address_map &map);
	void port_map_ebases(address_map &map);

	required_ioport_array<4> m_trackball;
};

class demndrgn_state : public astrocde_state
{
public:
	demndrgn_state(const machine_config &mconfig, device_type type, const char *tag)
		: astrocde_state(mconfig, type, tag)
		, m_joystick(*this, {"MOVEX", "MOVEY"})
	{ }

	void demndrgn(machine_config &config);
	DECLARE_CUSTOM_INPUT_MEMBER(joystick_r);
private:
	DECLARE_WRITE_LINE_MEMBER(input_select_w);
	DECLARE_WRITE8_MEMBER(sound_w);

	void port_map_16col_pattern_demndrgn(address_map &map);

	required_ioport_array<2> m_joystick;
};

class tenpindx_state : public astrocde_state
{
public:
	tenpindx_state(const machine_config &mconfig, device_type type, const char *tag) :
		astrocde_state(mconfig, type, tag),
		m_subcpu(*this, "sub"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void tenpindx(machine_config &config);
private:
	DECLARE_WRITE8_MEMBER(lamp_w);
	DECLARE_WRITE8_MEMBER(counter_w);
	DECLARE_WRITE8_MEMBER(lights_w);

	virtual void machine_start() override;

	void port_map_16col_pattern_tenpindx(address_map &map);
	void sub_io_map(address_map &map);
	void sub_map(address_map &map);

	required_device<z80_device> m_subcpu;
	output_finder<19> m_lamps;
};

#endif // MAME_INCLUDES_ASTROCDE_H
