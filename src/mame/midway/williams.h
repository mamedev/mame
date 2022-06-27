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
#include "machine/bankdev.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/hc55516.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class williams_state : public driver_device
{
public:
	williams_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_nvram(*this, "nvram"),
		m_videoram(*this, "videoram"),
		m_mainbank(*this, "mainbank"),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_watchdog(*this, "watchdog"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_paletteram(*this, "paletteram"),
		m_pia(*this, "pia_%u", 0U)
	{ }

	void lottofun(machine_config &config);
	void williams_base(machine_config &config);

	void init_stargate();
	void init_playball();
	void init_lottofun();
	void init_robotron();

	u8 port_0_49way_r();
	virtual u8 video_counter_r();
	virtual void watchdog_reset_w(u8 data);

	virtual TIMER_DEVICE_CALLBACK_MEMBER(va11_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(count240_callback);

	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void palette_init(palette_device &palette) const;

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

	enum
	{
		//controlbyte (0xCA00) bit definitions
		WMS_BLITTER_CONTROLBYTE_NO_EVEN = 0x80,
		WMS_BLITTER_CONTROLBYTE_NO_ODD = 0x40,
		WMS_BLITTER_CONTROLBYTE_SHIFT = 0x20,
		WMS_BLITTER_CONTROLBYTE_SOLID = 0x10,
		WMS_BLITTER_CONTROLBYTE_FOREGROUND_ONLY = 0x08,
		WMS_BLITTER_CONTROLBYTE_SLOW = 0x04, //2us blits instead of 1us
		WMS_BLITTER_CONTROLBYTE_DST_STRIDE_256 = 0x02,
		WMS_BLITTER_CONTROLBYTE_SRC_STRIDE_256 = 0x01
	};

	required_shared_ptr<uint8_t> m_nvram;
	required_shared_ptr<uint8_t> m_videoram;
	optional_memory_bank m_mainbank;
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
	virtual void vram_select_w(u8 data);
	virtual void cmos_w(offs_t offset, u8 data);
	void sinistar_vram_select_w(u8 data);
	void blitter_w(address_space &space, offs_t offset, u8 data);

	TIMER_CALLBACK_MEMBER(deferred_snd_cmd_w);

	void snd_cmd_w(u8 data);

	DECLARE_WRITE_LINE_MEMBER(lottofun_coin_lock_w);

	void state_save_register();
	void blitter_init(int blitter_config, const uint8_t *remap_prom);
	inline void blit_pixel(address_space &space, int dstaddr, int srcdata, int controlbyte);
	int blitter_core(address_space &space, int sstart, int dstart, int w, int h, int data);

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;
	optional_shared_ptr<uint8_t> m_paletteram;
	optional_device_array<pia6821_device, 4> m_pia;

	virtual void sound_map(address_map &map);
	void base_map(address_map &map);
};

class defender_state : public williams_state
{
public:
	defender_state(const machine_config &mconfig, device_type type, const char *tag) :
		williams_state(mconfig, type, tag),
		m_bankc000(*this, "bankc000")
	{ }

	void defender(machine_config &config);
	void jin(machine_config &config);

protected:
	virtual void driver_init() override;

	virtual void main_map(address_map &map);

	void video_control_w(u8 data);

	required_device<address_map_bank_device> m_bankc000;

private:
	virtual void machine_start() override { }
	virtual void machine_reset() override;

	void bankc000_map(address_map &map);
	virtual void sound_map(address_map &map) override;

	void bank_select_w(u8 data);
};

class defndjeu_state : public defender_state
{
public:
	defndjeu_state(const machine_config &mconfig, device_type type, const char *tag) :
		defender_state(mconfig, type, tag)
	{ }

private:
	virtual void driver_init() override;
};

class mayday_state : public defender_state
{
public:
	mayday_state(const machine_config &mconfig, device_type type, const char *tag) :
		defender_state(mconfig, type, tag)
	{ }

private:
	virtual void driver_init() override;

	virtual void main_map(address_map &map) override;

	uint8_t *m_protection = nullptr;
	u8 protection_r(offs_t offset);
};

class sinistar_state : public williams_state
{
public:
	sinistar_state(const machine_config &mconfig, device_type type, const char *tag) :
		williams_state(mconfig, type, tag)
	{ }

	void sinistar(machine_config &config);

private:
	virtual void driver_init() override;

	virtual void vram_select_w(u8 data) override;

	void main_map(address_map &map);
};

class bubbles_state : public williams_state
{
public:
	bubbles_state(const machine_config &mconfig, device_type type, const char *tag) :
		williams_state(mconfig, type, tag)
	{ }

	void bubbles(machine_config &config);

private:
	virtual void driver_init() override;

	void main_map(address_map &map);

	virtual void cmos_w(offs_t offset, u8 data) override;
};

class playball_state : public williams_state
{
public:
	playball_state(const machine_config &mconfig, device_type type, const char *tag) :
		williams_state(mconfig, type, tag)
	{ }

	void playball(machine_config &config);

private:
	virtual void driver_init() override;

	void snd_cmd_w(u8 data);
};

class williams_muxed_state : public williams_state
{
public:
	williams_muxed_state(const machine_config &mconfig, device_type type, const char *tag) :
		williams_state(mconfig, type, tag),
		m_mux0(*this, "mux_0"),
		m_mux1(*this, "mux_1")
	{ }

	void williams_muxed(machine_config &config);

	void init_splat();
	void init_joust();
	void init_alienar();
	void init_alienaru();

private:
	required_device<ls157_device> m_mux0;
	required_device<ls157_device> m_mux1;
};

class spdball_state : public williams_state
{
public:
	spdball_state(const machine_config &mconfig, device_type type, const char *tag) :
		williams_state(mconfig, type, tag)
	{ }

	void spdball(machine_config &config);

private:
	virtual void driver_init() override;

	void main_map(address_map &map);
};

class blaster_state : public williams_state
{
public:
	blaster_state(const machine_config &mconfig, device_type type, const char *tag) :
		williams_state(mconfig, type, tag),
		m_soundcpu_b(*this, "soundcpu_b"),
		m_bankb(*this, "blaster_bankb"),
		m_muxa(*this, "mux_a"),
		m_muxb(*this, "mux_b")
	{ }

	void blastkit(machine_config &config);
	void blaster(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void video_start() override;
	virtual void driver_init() override;

	optional_device<cpu_device> m_soundcpu_b;
	optional_memory_bank m_bankb;
	required_device<ls157_x2_device> m_muxa;
	optional_device<ls157_device> m_muxb;

	rgb_t m_color0;
	uint8_t m_video_control;
	uint8_t m_vram_bank;
	uint8_t m_rom_bank;

	virtual void vram_select_w(u8 data) override;
	void bank_select_w(u8 data);
	void remap_select_w(u8 data);
	void video_control_w(u8 data);
	TIMER_CALLBACK_MEMBER(deferred_snd_cmd_w);
	void snd_cmd_w(u8 data);

	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;

	inline void update_blaster_banking();

	void main_map(address_map &map);
	void sound2_map(address_map &map);
};

class williams2_state : public williams_state
{
public:
	williams2_state(const machine_config &mconfig, device_type type, const char *tag) :
		williams_state(mconfig, type, tag),
		m_bank8000(*this, "bank8000"),
		m_gfxdecode(*this, "gfxdecode"),
		m_tileram(*this, "williams2_tile"),
		m_gain(  { 0.25f, 0.25f, 0.25f }),
		m_offset({ 0.00f, 0.00f, 0.00f })
	{ }

	void williams2_base(machine_config &config);

	INPUT_CHANGED_MEMBER(rgb_gain)
	{
		if (param < 3)
			m_gain[param] = float(newval) / 100.0f;
		else
			m_offset[param - 3] = (float(newval) / 100.0f) - 1.0f;
		rebuild_palette();
	}

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	required_device<address_map_bank_device> m_bank8000;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint8_t> m_tileram;

	tilemap_t *m_bg_tilemap = nullptr;
	uint16_t m_tilemap_xscroll = 0;
	uint8_t m_fg_color = 0;
	std::array<float, 3> m_gain;
	std::array<float, 3> m_offset;

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
	void snd_cmd_w(u8 data);

	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;

	void bank8000_map(address_map &map);
	void common_map(address_map &map);
	virtual void sound_map(address_map &map) override;

	void video_control_w(u8 data);
};

class williams_d000_rom_state : public williams2_state
{
public:
	williams_d000_rom_state(const machine_config &mconfig, device_type type, const char *tag) :
		williams2_state(mconfig, type, tag)
	{ }

protected:
	void d000_map(address_map &map);
};

class williams_d000_ram_state : public williams2_state
{
public:
	williams_d000_ram_state(const machine_config &mconfig, device_type type, const char *tag) :
		williams2_state(mconfig, type, tag)
	{ }

protected:
	void d000_map(address_map &map);
};

class inferno_state : public williams_d000_ram_state
{
public:
	inferno_state(const machine_config &mconfig, device_type type, const char *tag) :
		williams_d000_ram_state(mconfig, type, tag),
		m_mux(*this, "mux")
	{ }

	void inferno(machine_config &config);

private:
	virtual void driver_init() override;

	required_device<ls157_device> m_mux;
};

class mysticm_state : public williams_d000_ram_state
{
public:
	mysticm_state(const machine_config &mconfig, device_type type, const char *tag) :
		williams_d000_ram_state(mconfig, type, tag)
	{
		// overwrite defaults for mysticm
		m_gain =   {   0.8f, 0.73f,  0.81f };
		m_offset = { -0.27f, 0.00f, -0.22f };
	}

	void mysticm(machine_config &config);

protected:
	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;

private:
	virtual void driver_init() override;

	virtual TILE_GET_INFO_MEMBER(get_tile_info) override;
	virtual void bg_select_w(u8 data) override;

	int color_decode(uint8_t base_col, int sig_J1, int y);

	uint8_t m_bg_color = 0;

};

class tshoot_state : public williams_d000_rom_state
{
public:
	tshoot_state(const machine_config &mconfig, device_type type, const char *tag) :
		williams_d000_rom_state(mconfig, type, tag),
		m_mux(*this, "mux"),
		m_gun(*this, {"GUNX", "GUNY"}),
		m_grenade_lamp(*this, "Grenade_lamp"),
		m_gun_lamp(*this, "Gun_lamp"),
		m_p1_gun_recoil(*this, "Player1_Gun_Recoil"),
		m_feather_blower(*this, "Feather_Blower")
	{ }

	void tshoot(machine_config &config);

	template <int P> DECLARE_CUSTOM_INPUT_MEMBER(gun_r);

private:
	virtual void machine_start() override;
	virtual void driver_init() override;

	DECLARE_WRITE_LINE_MEMBER(maxvol_w);
	void lamp_w(u8 data);

	required_device<ls157_device> m_mux;
	required_ioport_array<2> m_gun;
	output_finder<> m_grenade_lamp;
	output_finder<> m_gun_lamp;
	output_finder<> m_p1_gun_recoil;
	output_finder<> m_feather_blower;
};

class joust2_state : public williams_d000_rom_state
{
public:
	joust2_state(const machine_config &mconfig, device_type type, const char *tag) :
		williams_d000_rom_state(mconfig, type, tag),
		m_mux(*this, "mux"),
		m_bg(*this, "bg")
	{ }

	void joust2(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void driver_init() override;

	required_device<ls157_device> m_mux;
	required_device<s11_obg_device> m_bg;
	uint16_t m_current_sound_data = 0;

	virtual TILE_GET_INFO_MEMBER(get_tile_info) override;
	virtual void bg_select_w(u8 data) override;

	TIMER_CALLBACK_MEMBER(deferred_snd_cmd_w);
	void snd_cmd_w(u8 data);
	DECLARE_WRITE_LINE_MEMBER(pia_s11_bg_strobe_w);
};

/*----------- defined in video/williams.cpp -----------*/

#define WILLIAMS_BLITTER_NONE       0       /* no blitter */
#define WILLIAMS_BLITTER_SC1        1       /* Special Chip 1 blitter */
#define WILLIAMS_BLITTER_SC2        2       /* Special Chip 2 "bugfixed" blitter */

#define WILLIAMS_TILEMAP_MYSTICM    0       /* IC79 is a 74LS85 comparator */
#define WILLIAMS_TILEMAP_TSHOOT     1       /* IC79 is a 74LS157 selector jumpered to be enabled */
#define WILLIAMS_TILEMAP_JOUST2     2       /* IC79 is a 74LS157 selector jumpered to be disabled */

#endif // MAME_MIDWAY_WILLIAMS_H
