// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Mike Coates, Frank Palazzolo, Aaron Giles
/***************************************************************************

    Bally Astrocade-based hardware

***************************************************************************/
#ifndef MAME_MIDWAY_ASTROCDE_H
#define MAME_MIDWAY_ASTROCDE_H

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
	astrocde_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_votrax(*this, "votrax"),
		m_astrocade_sound(*this, "astrocade%u", 0),
		m_videoram(*this, "videoram"),
		m_protected_ram(*this, "protected_ram"),
		m_nvram(*this, "nvram"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_bank4000(*this, "bank4000"),
		m_bank8000(*this, "bank8000"),
		m_handle(*this, "P%uHANDLE", 1U)
	{ }

	required_device<cpu_device> m_maincpu;
	optional_device<votrax_sc01_device> m_votrax;
	optional_device_array<astrocade_io_device, 2> m_astrocade_sound;
	optional_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_protected_ram;
	optional_shared_ptr<uint8_t> m_nvram;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch;
	optional_device<address_map_bank_device> m_bank4000;
	optional_memory_bank m_bank8000;
	optional_ioport_array<4> m_handle;

	uint8_t m_video_config = 0U;
	uint8_t m_sparkle[4]{};
	char m_totalword[256]{};
	char *m_totalword_ptr = nullptr;
	char m_oldword[256]{};
	int m_plural = 0;
	uint8_t m_ram_write_enable = 0U;
	uint8_t m_input_select = 0U;
	std::unique_ptr<uint8_t[]> m_sparklestar{};
	uint8_t m_interrupt_enabl = 0U;
	uint8_t m_interrupt_vector = 0U;
	uint8_t m_interrupt_scanline = 0xff;
	uint8_t m_vertical_feedback = 0U;
	uint8_t m_horizontal_feedback = 0U;
	emu_timer *m_scanline_timer = nullptr;
	emu_timer *m_intoff_timer = nullptr;
	uint8_t m_colors[8]{};
	uint8_t m_colorsplit = 0U;
	uint8_t m_bgdata = 0U;
	uint8_t m_vblank = 0U;
	uint8_t m_video_mode = 0U;
	uint8_t m_funcgen_expand_color[2]{};
	uint8_t m_funcgen_control = 0U;
	uint8_t m_funcgen_expand_count = 0U;
	uint8_t m_funcgen_rotate_count = 0U;
	uint8_t m_funcgen_rotate_data[4]{};
	uint8_t m_funcgen_shift_prev_data = 0U;
	uint8_t m_funcgen_intercept = 0U;
	uint16_t m_pattern_source = 0U;
	uint8_t m_pattern_mode = 0U;
	uint16_t m_pattern_dest = 0U;
	uint8_t m_pattern_skip = 0U;
	uint8_t m_pattern_width = 0U;
	uint8_t m_pattern_height = 0U;
	std::unique_ptr<uint16_t[]> m_profpac_videoram{};
	uint16_t m_profpac_palette[16]{};
	uint8_t m_profpac_colormap[4]{};
	uint8_t m_profpac_intercept = 0U;
	uint8_t m_profpac_vispage = 0U;
	uint8_t m_profpac_readpage = 0U;
	uint8_t m_profpac_readshift = 0U;
	uint8_t m_profpac_writepage = 0U;
	uint8_t m_profpac_writemode = 0U;
	uint16_t m_profpac_writemask = 0U;
	uint8_t m_profpac_vw = 0U;

	void protected_ram_enable_w(uint8_t data);
	uint8_t protected_ram_r(offs_t offset);
	void protected_ram_w(offs_t offset, uint8_t data);
	uint8_t input_mux_r(offs_t offset);
	template<int Coin> void coin_counter_w(int state);
	template<int Bit> void sparkle_w(int state);
	void gorf_sound_switch_w(int state);
	void profpac_banksw_w(uint8_t data);
	void demndrgn_banksw_w(uint8_t data);
	uint8_t video_register_r(offs_t offset);
	void video_register_w(offs_t offset, uint8_t data);
	void astrocade_funcgen_w(address_space &space, offs_t offset, uint8_t data);
	void expand_register_w(uint8_t data);
	void astrocade_pattern_board_w(offs_t offset, uint8_t data);
	void profpac_page_select_w(uint8_t data);
	uint8_t profpac_intercept_r();
	void profpac_screenram_ctrl_w(offs_t offset, uint8_t data);
	uint8_t profpac_videoram_r(offs_t offset);
	void profpac_videoram_w(offs_t offset, uint8_t data);
	DECLARE_INPUT_CHANGED_MEMBER(spacezap_monitor);
	void lightpen_trigger_w(int state);
	void init_profpac();
	void init_spacezap();
	void init_robby();
	void init_wow();
	void init_tenpindx();
	void init_seawolf2();
	void init_demndrgn();
	void init_ebases();
	void init_gorf();
	void astrocade_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(profpac);
	void profpac_palette(palette_device &palette) const;
	uint32_t screen_update_astrocde(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_profpac(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(interrupt_off);
	TIMER_CALLBACK_MEMBER(scanline_callback);
	inline int mame_vpos_to_astrocade_vpos(int scanline);
	void init_savestate();
	void astrocade_trigger_lightpen(uint8_t vfeedback, uint8_t hfeedback);
	inline void increment_source(uint8_t curwidth, uint8_t *u13ff);
	inline void increment_dest(uint8_t curwidth);
	void execute_blit();
	void init_sparklestar();

	void votrax_speech_w(uint8_t data);
	int votrax_speech_status_r();

	void astrocade_base(machine_config &config);
	void astrocade_16color_base(machine_config &config);
	void astrocade_mono_sound(machine_config &config);
	void astrocade_stereo_sound(machine_config &config);
	void spacezap(machine_config &config);
	void gorf(machine_config &config);
	void profpac(machine_config &config);
	void robby(machine_config &config);
	void wow(machine_config &config);
	void bank4000_map(address_map &map) ATTR_COLD;
	void demndrgn_map(address_map &map) ATTR_COLD;
	void port_map(address_map &map) ATTR_COLD;
	void port_map_16col_pattern(address_map &map) ATTR_COLD;
	void port_map_16col_pattern_nosound(address_map &map) ATTR_COLD;
	void port_map_mono_pattern(address_map &map) ATTR_COLD;
	void port_map_stereo_pattern(address_map &map) ATTR_COLD;
	void profpac_bank4000_map(address_map &map) ATTR_COLD;
	void profpac_map(address_map &map) ATTR_COLD;
	void robby_map(address_map &map) ATTR_COLD;
	void seawolf2_map(address_map &map) ATTR_COLD;
	void spacezap_map(address_map &map) ATTR_COLD;
	void wow_map(address_map &map) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
};

class seawolf2_state : public astrocde_state
{
public:
	seawolf2_state(const machine_config &mconfig, device_type type, const char *tag) :
		astrocde_state(mconfig, type, tag),
		m_samples(*this, "samples")
	{ }

	void seawolf2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void sound_1_w(uint8_t data);
	void sound_2_w(uint8_t data);

	void port_map_discrete(address_map &map) ATTR_COLD;

	required_device<samples_device> m_samples;
	uint8_t m_port_1_last = 0U;
	uint8_t m_port_2_last = 0U;
};

class ebases_state : public astrocde_state
{
public:
	ebases_state(const machine_config &mconfig, device_type type, const char *tag) :
		astrocde_state(mconfig, type, tag),
		m_trackball(*this, {"TRACKX2", "TRACKY2", "TRACKX1", "TRACKY1"})
	{ }

	void ebases(machine_config &config);
	ioport_value trackball_r();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void trackball_select_w(uint8_t data);
	void coin_w(uint8_t data);

	void ebases_map(address_map &map) ATTR_COLD;
	void port_map_ebases(address_map &map) ATTR_COLD;

	required_ioport_array<4> m_trackball;
	uint8_t m_trackball_last = 0U;
};

class demndrgn_state : public astrocde_state
{
public:
	demndrgn_state(const machine_config &mconfig, device_type type, const char *tag) :
		astrocde_state(mconfig, type, tag),
		m_trackball(*this, {"MOVEX", "MOVEY"})
	{ }

	void demndrgn(machine_config &config);
	ioport_value trackball_r();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void input_select_w(int state);
	void sound_w(uint8_t data);
	void trackball_reset_w(uint8_t data);

	void port_map_16col_pattern_demndrgn(address_map &map) ATTR_COLD;

	required_ioport_array<2> m_trackball;
	uint8_t m_trackball_last = 0U;
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
	void lamp_w(offs_t offset, uint8_t data);
	void counter_w(uint8_t data);
	void lights_w(uint8_t data);

	virtual void machine_start() override ATTR_COLD;

	void port_map_16col_pattern_tenpindx(address_map &map) ATTR_COLD;
	void sub_io_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_subcpu;
	output_finder<19> m_lamps;
};

#endif // MAME_MIDWAY_ASTROCDE_H
