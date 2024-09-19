// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Bryan McPhail
/*************************************************************************

    SNK/Alpha 68000 based games

*************************************************************************/
#ifndef MAME_ALPHA_ALPHA68K_H
#define MAME_ALPHA_ALPHA68K_H

#pragma once

#include "alpha68k_palette.h"
#include "snk68_spr.h"

#include "cpu/m68000/m68000.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/ymopn.h"
#include "sound/ymopl.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


class alpha68k_state : public driver_device
{
public:
	alpha68k_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_outlatch(*this, "outlatch"),
		m_soundlatch(*this, "soundlatch"),
		m_shared_ram(*this, "shared_ram"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_in(*this, "IN%u", 0U),
		m_audiobank(*this, "audiobank")
	{ }

protected:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	optional_device<ls259_device> m_outlatch;
	required_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	optional_shared_ptr<u16> m_shared_ram;
	required_shared_ptr<u16> m_spriteram;
	optional_shared_ptr<u16> m_videoram;

	optional_ioport_array<7> m_in;
	optional_memory_bank m_audiobank;

	int           m_flipscreen = 0;

	// MCU sims
	void alpha_microcontroller_w(offs_t offset, u16 data, u16 mem_mask);
	int           m_invert_controls = 0;
	int           m_microcontroller_id = 0;
	unsigned      m_game_id = 0U;  // see below
	unsigned      m_deposits1 = 0U;
	unsigned      m_deposits2 = 0U;
	unsigned      m_credits = 0U;
	unsigned      m_coinvalue = 0U;
	int           m_coin_id = 0;
	unsigned      m_microcontroller_data = 0U;
	unsigned      m_trigstate = 0U;
	int           m_latch = 0;

	void set_screen_raw_params(machine_config &config);

	DECLARE_MACHINE_START(common);
	DECLARE_MACHINE_RESET(common);

};

class alpha68k_II_state : public alpha68k_state
{
public:
	alpha68k_II_state(const machine_config &mconfig, device_type type, const char *tag)
		: alpha68k_state(mconfig, type, tag)
		, m_sprites(*this, "sprites")
		, m_palette(*this, "palette")
	{}

	void alpha68k_II(machine_config &config);
	void btlfieldb(machine_config &config);

	void init_skysoldr();
	void init_timesold();
	void init_timesold1();
	void init_btlfield();
	void init_btlfieldb();
protected:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<snk68_spr_device> m_sprites;
	required_device<alpha68k_palette_device> m_palette;

	void base_config(machine_config &config);
	DECLARE_VIDEO_START(alpha68k);
	void video_config(machine_config &config, u16 num_pens);
	void tile_callback(int &tile, int& fx, int& fy, int& region);
	void tile_callback_noflipx(int &tile, int& fx, int& fy, int& region);
	void tile_callback_noflipy(int &tile, int& fx, int& fy, int& region);

	INTERRUPT_GEN_MEMBER(sound_nmi);
	void sound_bank_w(u8 data);
	void flipscreen_w(int flip);
	void porta_w(u8 data);
	u8       m_sound_nmi_mask = 0U;
	u8       m_sound_pa_latch = 0U;

	DECLARE_MACHINE_START(alpha68k_II);
	DECLARE_MACHINE_RESET(alpha68k_II);
	void video_bank_w(u8 data);
	void alpha68k_II_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void sound_portmap(address_map &map) ATTR_COLD;

	u16 alpha_II_trigger_r(offs_t offset);

	/* video-related */
	tilemap_t     *m_fix_tilemap = nullptr;
	int           m_bank_base = 0;

	void outlatch_w(offs_t offset, u8 data = 0);
	u16 control_1_r();
	u16 control_2_r();
	u16 control_3_r();
	u16 control_4_r();
	void videoram_w(offs_t offset, u16 data);
	void video_control2_w(int state);
	void video_control3_w(int state);
private:
	TILE_GET_INFO_MEMBER(get_tile_info);
};

class goldmedal_II_state : public alpha68k_II_state
{
public:
	goldmedal_II_state(const machine_config &mconfig, device_type type, const char *tag)
		: alpha68k_II_state(mconfig, type, tag)
	{}

	void init_goldmedl();
	void goldmedal(machine_config &config);
};

class alpha68k_III_state : public alpha68k_II_state
{
public:
	alpha68k_III_state(const machine_config &mconfig, device_type type, const char *tag)
		: alpha68k_II_state(mconfig, type, tag)
	{}

	void alpha68k_III(machine_config &config);
protected:
	DECLARE_MACHINE_START(alpha68k_V);
	DECLARE_MACHINE_RESET(alpha68k_V);
	void alpha68k_III_map(address_map &map) ATTR_COLD;
	void alpha68k_V_map(address_map &map) ATTR_COLD;
	u16 alpha_V_trigger_r(offs_t offset);
};

class goldmedal_III_state : public alpha68k_III_state
{
public:
	goldmedal_III_state(const machine_config &mconfig, device_type type, const char *tag)
		: alpha68k_III_state(mconfig, type, tag)
	{}

	void init_goldmedla();
	void goldmedal(machine_config &config);
};

class alpha68k_V_state : public alpha68k_III_state
{
public:
	alpha68k_V_state(const machine_config &mconfig, device_type type, const char *tag)
		: alpha68k_III_state(mconfig, type, tag)
	{}

	void init_skyadvnt();
	void init_skyadvntu();
	void init_sbasebal();
	void init_sbasebalj();
	void init_gangwarsu();
	void init_gangwars();
	void alpha68k_V(machine_config &config);
};

class skyadventure_state : public alpha68k_V_state
{
public:
	skyadventure_state(const machine_config &mconfig, device_type type, const char *tag)
		: alpha68k_V_state(mconfig, type, tag)
	{}

	void skyadventure(machine_config &config);
};

class gangwars_state : public alpha68k_V_state
{
public:
	gangwars_state(const machine_config &mconfig, device_type type, const char *tag)
		: alpha68k_V_state(mconfig, type, tag)
	{}

	void gangwars(machine_config &config);
};

/*
 * Base class for HWs with 4bpp PROMs for colors
 */

class alpha68k_prom_state : public alpha68k_state
{
public:
	alpha68k_prom_state(const machine_config &mconfig, device_type type, const char *tag)
		: alpha68k_state(mconfig, type, tag),
		m_palette(*this, "palette"),
		m_color_proms(*this, "color_proms")
	{}

protected:
	void palette_init(palette_device &palette) const;

	required_device<palette_device> m_palette;
	optional_region_ptr<u8> m_color_proms;
};

/*
 *
 * Alpha68k N games
 *
 */
class alpha68k_N_state : public alpha68k_prom_state
{
public:
	alpha68k_N_state(const machine_config &mconfig, device_type type, const char *tag)
		: alpha68k_prom_state(mconfig, type, tag)
	{}

protected:
	void main_map(address_map &map) ATTR_COLD;

	void base_config(machine_config &config);
	void video_config(machine_config &config, u8 tile_transchar, u8 tile_bankshift, bool is_super_stingray);

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int c, int d);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	u16 kyros_alpha_trigger_r(offs_t offset);
	void sound_map(address_map &map) ATTR_COLD;
	void sound_iomap(address_map &map) ATTR_COLD;

private:
	u16 m_tile_transchar = 0U;
	int m_tile_bankshift = 0;
	bool m_is_super_stingray = false;
};

class sstingray_state : public alpha68k_N_state
{
public:
	sstingray_state(const machine_config &mconfig, device_type type, const char *tag)
		: alpha68k_N_state(mconfig, type, tag)
		, m_alpha8511(*this, "alpha8511")
	{}

	void init_sstingry();
	void sstingry(machine_config &config);

private:
	u8 alpha8511_command_r(offs_t offset);
	void alpha8511_command_w(offs_t offset, u8 data);
	TIMER_CALLBACK_MEMBER(alpha8511_sync);
	u8 alpha8511_bus_r();
	void alpha8511_bus_w(u8 data);
	u8 alpha8511_address_r();
	u8 alpha8511_rw_r();
	void alpha8511_control_w(u8 data);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	required_device<mcs48_cpu_device> m_alpha8511;
	u8 m_alpha8511_address = 0U;
	u8 m_alpha8511_control = 0U;
	bool m_alpha8511_read_mode = 0;
	emu_timer *m_alpha8511_sync_timer = 0;
};

class kyros_state : public alpha68k_N_state
{
public:
	kyros_state(const machine_config &mconfig, device_type type, const char *tag)
		: alpha68k_N_state(mconfig, type, tag)
	{}

	void kyros(machine_config &config);
	void init_kyros();
};

class jongbou_state : public alpha68k_N_state
{
public:
	jongbou_state(const machine_config &mconfig, device_type type, const char *tag)
		: alpha68k_N_state(mconfig, type, tag)
	{}

	void jongbou(machine_config &config);
	void init_jongbou();
	void init_jongbou2();

private:
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void sound_iomap(address_map &map) ATTR_COLD;
	u16 dial_inputs_r();
};

/*
 *
 * Alpha68k I games
 *
 */
class alpha68k_I_state : public alpha68k_prom_state
{
public:
	alpha68k_I_state(const machine_config &mconfig, device_type type, const char *tag)
		: alpha68k_prom_state(mconfig, type, tag)
	{}


protected:
	void base_config(machine_config &config);
	void video_config(machine_config &config, int yshift);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int c, int d);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	int m_yshift = 0;
};

class paddlemania_state : public alpha68k_I_state
{
public:
	paddlemania_state(const machine_config &mconfig, device_type type, const char *tag)
		: alpha68k_I_state(mconfig, type, tag)
	{}

	void paddlema(machine_config &config);
	void init_paddlema();

private:
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};

class thenextspace_state : public alpha68k_I_state
{
public:
	thenextspace_state(const machine_config &mconfig, device_type type, const char *tag)
		: alpha68k_I_state(mconfig, type, tag)
	{}

	void tnextspc(machine_config &config);
	void init_tnextspc();

private:
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void sound_iomap(address_map &map) ATTR_COLD;

	void tnextspc_coin_counters_w(offs_t offset, u16 data);
	void tnextspc_unknown_w(offs_t offset, u16 data);
	void tnextspc_soundlatch_w(u8 data);
	u16 sound_cpu_r();
};

/* game_id - used to deal with a few game specific situations */
enum
{
	ALPHA68K_BTLFIELDB = 1,     // used in alpha_II_trigger_r
	ALPHA68K_JONGBOU,           // used in kyros_alpha_trigger_r & kyros_draw_sprites
	ALPHA68K_KYROS          // used in kyros_draw_sprites
};

#define ALPHA68K_PLAYER_INPUT_LSB( player, button3, start, active ) \
	PORT_BIT( 0x0001, active, IPT_JOYSTICK_UP    ) PORT_PLAYER(player) \
	PORT_BIT( 0x0002, active, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(player) \
	PORT_BIT( 0x0004, active, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(player) \
	PORT_BIT( 0x0008, active, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(player) \
	PORT_BIT( 0x0010, active, IPT_BUTTON1        ) PORT_PLAYER(player) \
	PORT_BIT( 0x0020, active, IPT_BUTTON2        ) PORT_PLAYER(player) \
	PORT_BIT( 0x0040, active, button3            ) PORT_PLAYER(player) \
	PORT_BIT( 0x0080, active, start )

#define ALPHA68K_PLAYER_INPUT_MSB( player, button3, start, active ) \
	PORT_BIT( 0x0100, active, IPT_JOYSTICK_UP    ) PORT_PLAYER(player) \
	PORT_BIT( 0x0200, active, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(player) \
	PORT_BIT( 0x0400, active, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(player) \
	PORT_BIT( 0x0800, active, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(player) \
	PORT_BIT( 0x1000, active, IPT_BUTTON1        ) PORT_PLAYER(player) \
	PORT_BIT( 0x2000, active, IPT_BUTTON2        ) PORT_PLAYER(player) \
	PORT_BIT( 0x4000, active, button3            ) PORT_PLAYER(player) \
	PORT_BIT( 0x8000, active, start )

#define ALPHA68K_MCU \
	PORT_START("IN2")  /* Coin input to microcontroller */\
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )\
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )

#endif // MAME_ALPHA_ALPHA68K_H
