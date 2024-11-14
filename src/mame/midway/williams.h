// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Driver for early Williams games

**************************************************************************/
#ifndef MAME_MIDWAY_WILLIAMS_H
#define MAME_MIDWAY_WILLIAMS_H

#pragma once

#include "s11c_bg.h"
#include "williamssound.h"

#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "machine/74157.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/hc55516.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


// base Williams hardware
class williams_state : public driver_device
{
public:
	williams_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_watchdog(*this, "watchdog"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_paletteram(*this, "paletteram"),
		m_pia(*this, "pia_%u", 0U),
		m_nvram(*this, "nvram"),
		m_videoram(*this, "videoram"),
		m_rom_view(*this, "rom_view"),
		m_49way_x(*this, "49WAYX"),
		m_49way_y(*this, "49WAYY")
	{ }

	void williams_b0(machine_config &config);
	void williams_b1(machine_config &config);
	void williams_b2(machine_config &config);

	void joust(machine_config &config);
	void bubbles(machine_config &config);
	void sinistar_upright(machine_config &config);
	void sinistar_cockpit(machine_config &config);
	void splat(machine_config &config);
	void playball(machine_config &config);
	void spdball(machine_config &config);
	void alienar(machine_config &config);
	void lottofun(machine_config &config);

	virtual u8 video_counter_r();
	virtual void watchdog_reset_w(u8 data);

	virtual TIMER_DEVICE_CALLBACK_MEMBER(va11_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(count240_callback);

	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void palette_init(palette_device &palette) const;

protected:
	// blitter type
	enum
	{
		WILLIAMS_BLITTER_NONE = 0, // no blitter
		WILLIAMS_BLITTER_SC1 = 1,  // Special Chip 1 blitter
		WILLIAMS_BLITTER_SC2 = 2   // Special Chip 2 "bugfixed" blitter
	};

	// controlbyte (0xCA00) bit definitions
	enum
	{
		WMS_BLITTER_CONTROLBYTE_NO_EVEN = 0x80,
		WMS_BLITTER_CONTROLBYTE_NO_ODD = 0x40,
		WMS_BLITTER_CONTROLBYTE_SHIFT = 0x20,
		WMS_BLITTER_CONTROLBYTE_SOLID = 0x10,
		WMS_BLITTER_CONTROLBYTE_FOREGROUND_ONLY = 0x08,
		WMS_BLITTER_CONTROLBYTE_SLOW = 0x04, // 2us blits instead of 1us
		WMS_BLITTER_CONTROLBYTE_DST_STRIDE_256 = 0x02,
		WMS_BLITTER_CONTROLBYTE_SRC_STRIDE_256 = 0x01
	};

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;
	optional_shared_ptr<uint8_t> m_paletteram;
	optional_device_array<pia6821_device, 4> m_pia;

	required_shared_ptr<uint8_t> m_nvram;
	required_shared_ptr<uint8_t> m_videoram;

	memory_view m_rom_view;

	optional_ioport m_49way_x;
	optional_ioport m_49way_y;

	uint8_t m_blitter_config;
	uint16_t m_blitter_clip_address;
	uint8_t m_blitter_window_enable;
	uint8_t m_cocktail;
	std::unique_ptr<rgb_t[]> m_palette_lookup;
	uint8_t m_blitterram[8];
	uint8_t m_blitter_xor;
	uint8_t m_blitter_remap_index;
	const uint8_t *m_blitter_remap;
	std::unique_ptr<uint8_t[]> m_blitter_remap_lookup;

	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	u8 port_0_49way_r();
	void vram_select_w(u8 data);
	void sinistar_vram_select_w(u8 data);
	void cmos_4bit_w(offs_t offset, u8 data);
	void blitter_w(address_space &space, offs_t offset, u8 data);

	template <unsigned A, unsigned... B>
	TIMER_CALLBACK_MEMBER(deferred_snd_cmd_w);
	virtual void snd_cmd_w(u8 data);
	void playball_snd_cmd_w(u8 data);
	void cockpit_snd_cmd_w(u8 data);

	void state_save_register();
	void blitter_init(int blitter_config, const uint8_t *remap_prom);
	inline void blit_pixel(address_space &space, int dstaddr, int srcdata, int controlbyte);
	int blitter_core(address_space &space, int sstart, int dstart, int w, int h, int data);

	void williams_base(machine_config &config);
	void williams_muxed(machine_config &config);

	void main_map(address_map &map) ATTR_COLD;
	void bubbles_main_map(address_map &map) ATTR_COLD;
	void sinistar_main_map(address_map &map) ATTR_COLD;
	void spdball_main_map(address_map &map) ATTR_COLD;
	void alienar_main_map(address_map &map) ATTR_COLD;
	virtual void sound_map(address_map &map) ATTR_COLD;
	void sound2_map(address_map &map) ATTR_COLD; // for Blaster and Sinistar cockpit
};

// Defender
class defender_state : public williams_state
{
public:
	defender_state(const machine_config &mconfig, device_type type, const char *tag) :
		williams_state(mconfig, type, tag)
	{ }

	void defender(machine_config &config);
	void defender_6802snd(machine_config &config);
	void jin(machine_config &config);

	void init_defndjeu();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void defender_main_map(address_map &map) ATTR_COLD;
	void defender_sound_map(address_map &map) ATTR_COLD;
	void defender_sound_map_6802(address_map &map) ATTR_COLD;

private:
	void video_control_w(u8 data);
	void bank_select_w(u8 data);
};

// Hoei Mayday: custom protection
class mayday_state : public defender_state
{
public:
	mayday_state(const machine_config &mconfig, device_type type, const char *tag) :
		defender_state(mconfig, type, tag)
	{ }

	void mayday(machine_config &config);

private:
	void mayday_main_map(address_map &map) ATTR_COLD;

	u8 protection_r(offs_t offset);
};

// Conquest: flywheel controller
class conquest_state : public williams_state
{
public:
	conquest_state(const machine_config &mconfig, device_type type, const char *tag) :
		williams_state(mconfig, type, tag),
		m_dial(*this, "DIAL")
	{ }

	ioport_value dial0_r() { return m_dial->read() & 3; }
	ioport_value dial1_r() { return m_dial->read() >> 2 & 3; }

private:
	required_ioport m_dial;
};

// Blaster: extra sound hardware
class blaster_state : public williams_state
{
public:
	blaster_state(const machine_config &mconfig, device_type type, const char *tag) :
		williams_state(mconfig, type, tag),
		m_muxa(*this, "mux_a"),
		m_mainbank(*this, "mainbank")
	{ }

	void blastkit(machine_config &config);
	void blaster(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<ls157_x2_device> m_muxa;
	required_memory_bank m_mainbank;

	rgb_t m_color0;
	uint8_t m_video_control;

	void blaster_vram_select_w(u8 data);
	void bank_select_w(u8 data);
	void remap_select_w(u8 data);
	void video_control_w(u8 data);
	TIMER_CALLBACK_MEMBER(deferred_snd_cmd_w);
	void blaster_snd_cmd_w(u8 data);

	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;

	void blaster_main_map(address_map &map) ATTR_COLD;
};

// base Williams 2nd gen hardware
class williams2_state : public williams_state
{
public:
	williams2_state(const machine_config &mconfig, device_type type, const char *tag) :
		williams_state(mconfig, type, tag),
		m_gfxdecode(*this, "gfxdecode"),
		m_tileram(*this, "tileram"),
		m_mainbank(*this, "mainbank"),
		m_palette_view(*this, "palette_view"),
		m_gain(  { 0.25f, 0.25f, 0.25f }),
		m_offset({ 0.00f, 0.00f, 0.00f })
	{ }

	void inferno(machine_config &config);

	INPUT_CHANGED_MEMBER(rgb_gain)
	{
		if (param < 3)
			m_gain[param] = float(newval) / 100.0f;
		else
			m_offset[param - 3] = (float(newval) / 100.0f) - 1.0f;
		rebuild_palette();
	}

protected:
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint8_t> m_tileram;
	required_memory_bank m_mainbank;

	memory_view m_palette_view;

	tilemap_t *m_bg_tilemap = nullptr;
	uint16_t m_tilemap_xscroll = 0;
	uint8_t m_fg_color = 0;
	std::array<float, 3> m_gain;
	std::array<float, 3> m_offset;

	void williams2_base(machine_config &config);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	virtual u8 video_counter_r() override;

	virtual TILE_GET_INFO_MEMBER(get_tile_info);
	void bank_select_w(u8 data);
	virtual void watchdog_reset_w(u8 data) override;
	void segments_w(u8 data);

	rgb_t calc_col(uint16_t lo, uint16_t hi);
	void paletteram_w(offs_t offset, u8 data);
	void rebuild_palette();
	void fg_select_w(u8 data);
	virtual void bg_select_w(u8 data);
	void tileram_w(offs_t offset, u8 data);
	void xscroll_low_w(u8 data);
	void xscroll_high_w(u8 data);
	void blit_window_enable_w(u8 data);
	virtual TIMER_DEVICE_CALLBACK_MEMBER(va11_callback) override;
	TIMER_DEVICE_CALLBACK_MEMBER(endscreen_callback);
	TIMER_CALLBACK_MEMBER(deferred_snd_cmd_w);
	virtual void snd_cmd_w(u8 data) override;

	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;

	void common_map(address_map &map) ATTR_COLD;
	void d000_rom_map(address_map &map) ATTR_COLD;
	void d000_ram_map(address_map &map) ATTR_COLD;
	virtual void sound_map(address_map &map) override ATTR_COLD;

	void video_control_w(u8 data);
};

// Mystic Marathon
class mysticm_state : public williams2_state
{
public:
	mysticm_state(const machine_config &mconfig, device_type type, const char *tag) :
		williams2_state(mconfig, type, tag)
	{
		// overwrite defaults for mysticm
		m_gain =   {   0.8f, 0.73f,  0.81f };
		m_offset = { -0.27f, 0.00f, -0.22f };
	}

	void mysticm(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;

private:
	virtual TILE_GET_INFO_MEMBER(get_tile_info) override;
	virtual void bg_select_w(u8 data) override;

	int color_decode(uint8_t base_col, int sig_J1, int y);

	uint8_t m_bg_color = 0;
};

// Turkey Shoot
class tshoot_state : public williams2_state
{
public:
	tshoot_state(const machine_config &mconfig, device_type type, const char *tag) :
		williams2_state(mconfig, type, tag),
		m_mux(*this, "mux"),
		m_gun(*this, {"GUNX", "GUNY"}),
		m_grenade_lamp(*this, "Grenade_lamp"),
		m_gun_lamp(*this, "Gun_lamp"),
		m_p1_gun_recoil(*this, "Player1_Gun_Recoil"),
		m_feather_blower(*this, "Feather_Blower")
	{ }

	void tshoot(machine_config &config);

	template <int P> ioport_value gun_r();

private:
	virtual void machine_start() override ATTR_COLD;

	void maxvol_w(int state);
	void lamp_w(u8 data);

	required_device<ls157_device> m_mux;
	required_ioport_array<2> m_gun;
	output_finder<> m_grenade_lamp;
	output_finder<> m_gun_lamp;
	output_finder<> m_p1_gun_recoil;
	output_finder<> m_feather_blower;
};

// Joust 2
class joust2_state : public williams2_state
{
public:
	joust2_state(const machine_config &mconfig, device_type type, const char *tag) :
		williams2_state(mconfig, type, tag),
		m_mux(*this, "mux"),
		m_bg(*this, "bg")
	{ }

	void joust2(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;

	required_device<ls157_device> m_mux;
	required_device<s11_obg_device> m_bg;
	uint16_t m_current_sound_data = 0;

	virtual TILE_GET_INFO_MEMBER(get_tile_info) override;
	virtual void bg_select_w(u8 data) override;

	TIMER_CALLBACK_MEMBER(deferred_snd_cmd_w);
	virtual void snd_cmd_w(u8 data) override;
	void pia_s11_bg_strobe_w(int state);
};


#endif // MAME_MIDWAY_WILLIAMS_H
