// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Tormod Tjaberg, Mirko Buffoni,Lee Taylor, Valerio Verrando, Zsolt Vasvari
// thanks-to:Michael Strutts, Marco Cassili
/***************************************************************************

    Midway 8080-based black and white hardware

****************************************************************************/
#ifndef MAME_INCLUDES_MW8080BW_H
#define MAME_INCLUDES_MW8080BW_H

#pragma once

#include "audio/mw8080bw.h"

#include "machine/mb14241.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/discrete.h"
#include "screen.h"


#define MW8080BW_MASTER_CLOCK             (19968000.0)
#define MW8080BW_CPU_CLOCK                (MW8080BW_MASTER_CLOCK / 10)
#define MW8080BW_PIXEL_CLOCK              (MW8080BW_MASTER_CLOCK / 4)
#define MW8080BW_HTOTAL                   (0x140)
#define MW8080BW_HBEND                    (0x000)
#define MW8080BW_HBSTART                  (0x100)
#define MW8080BW_VTOTAL                   (0x106)
#define MW8080BW_VBEND                    (0x000)
#define MW8080BW_VBSTART                  (0x0e0)
#define MW8080BW_VCOUNTER_START_NO_VBLANK (0x020)
#define MW8080BW_VCOUNTER_START_VBLANK    (0x0da)
#define MW8080BW_INT_TRIGGER_COUNT_1      (0x080)
#define MW8080BW_INT_TRIGGER_VBLANK_1     (0)
#define MW8080BW_INT_TRIGGER_COUNT_2      MW8080BW_VCOUNTER_START_VBLANK
#define MW8080BW_INT_TRIGGER_VBLANK_2     (1)
#define MW8080BW_60HZ                     (MW8080BW_PIXEL_CLOCK / MW8080BW_HTOTAL / MW8080BW_VTOTAL)

// +4 is added to HBSTART because the hardware displays that many pixels after setting HBLANK
#define MW8080BW_HPIXCOUNT                (MW8080BW_HBSTART + 4)


class mw8080bw_state : public driver_device
{
public:
	mw8080bw_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_mb14241(*this,"mb14241"),
		m_watchdog(*this, "watchdog"),
		m_main_ram(*this, "main_ram"),
		m_discrete(*this, "discrete"),
		m_screen(*this, "screen"),
		m_int_enable(true)
	{ }

	void blueshrk(machine_config &config);
	void bowler(machine_config &config);
	void checkmat(machine_config &config);
	void dogpatch(machine_config &config);
	void invad2ct(machine_config &config);
	void invaders(machine_config &config);
	void maze(machine_config &config);
	void mw8080bw_root(machine_config &config);
	void phantom2(machine_config &config);
	void shuffle(machine_config &config);
	void tornbase(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(direct_coin_count);

	DECLARE_CUSTOM_INPUT_MEMBER(tornbase_hit_left_input_r);
	DECLARE_CUSTOM_INPUT_MEMBER(tornbase_hit_right_input_r);
	DECLARE_CUSTOM_INPUT_MEMBER(tornbase_pitch_left_input_r);
	DECLARE_CUSTOM_INPUT_MEMBER(tornbase_pitch_right_input_r);
	DECLARE_CUSTOM_INPUT_MEMBER(tornbase_score_input_r);
	DECLARE_CUSTOM_INPUT_MEMBER(blueshrk_coin_input_r);

	DECLARE_CUSTOM_INPUT_MEMBER(invaders_sw6_sw7_r);
	DECLARE_CUSTOM_INPUT_MEMBER(invaders_sw5_r);
	DECLARE_CUSTOM_INPUT_MEMBER(invaders_in0_control_r);
	DECLARE_CUSTOM_INPUT_MEMBER(invaders_in1_control_r);
	DECLARE_CUSTOM_INPUT_MEMBER(invaders_in2_control_r);

	DECLARE_MACHINE_RESET(mw8080bw);

	IRQ_CALLBACK_MEMBER(interrupt_vector);

protected:
	virtual void machine_start() override;

	DECLARE_WRITE_LINE_MEMBER(int_enable_w);

	u8 mw8080bw_shift_result_rev_r();

	int invaders_is_cabinet_cocktail();

	uint32_t screen_update_invaders(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_mw8080bw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	// device/memory pointers
	required_device<cpu_device> m_maincpu;
	optional_device<mb14241_device> m_mb14241;
	optional_device<watchdog_timer_device> m_watchdog;
	required_shared_ptr<uint8_t> m_main_ram;
	optional_device<discrete_sound_device> m_discrete;
	required_device<screen_device> m_screen;

	// misc game specific
	uint8_t       m_flip_screen;

private:
	// misc game specific
	uint16_t      m_phantom2_cloud_counter;
	uint8_t       m_maze_tone_timing_state;   // output of IC C1, pin 5

	// timers
	emu_timer   *m_interrupt_timer;
	emu_timer   *m_maze_tone_timer;

	attotime m_interrupt_time;

	bool m_int_enable;

	void tornbase_io_w(offs_t offset, uint8_t data);
	void maze_coin_counter_w(uint8_t data);
	void maze_io_w(offs_t offset, uint8_t data);
	void checkmat_io_w(offs_t offset, uint8_t data);
	uint8_t bowler_shift_result_r();
	void bowler_lights_1_w(uint8_t data);
	void bowler_lights_2_w(uint8_t data);
	void bowler_audio_2_w(uint8_t data);
	void bowler_audio_3_w(uint8_t data);
	void bowler_audio_4_w(uint8_t data);
	void bowler_audio_5_w(uint8_t data);
	void bowler_audio_6_w(uint8_t data);
	DECLARE_MACHINE_START(maze);
	DECLARE_MACHINE_START(phantom2);
	DECLARE_MACHINE_START(invaders);
	uint32_t screen_update_phantom2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_phantom2);
	TIMER_CALLBACK_MEMBER(maze_tone_timing_timer_callback);
	TIMER_CALLBACK_MEMBER(interrupt_trigger);
	void tornbase_audio_w(uint8_t data);
	void checkmat_audio_w(uint8_t data);
	void shuffle_audio_1_w(uint8_t data);
	void shuffle_audio_2_w(uint8_t data);
	void bowler_audio_1_w(uint8_t data);
	void blueshrk_audio_w(uint8_t data);
	void maze_update_discrete();
	void maze_write_discrete(uint8_t maze_tone_timing_state);
	uint8_t vpos_to_vysnc_chain_counter(int vpos);
	int vysnc_chain_counter_to_vpos(uint8_t counter, int vblank);
	void mw8080bw_create_interrupt_timer();
	void mw8080bw_start_interrupt_timer();
	uint8_t tornbase_get_cabinet_type();

	void blueshrk_audio(machine_config &config);
	void bowler_audio(machine_config &config);
	void checkmat_audio(machine_config &config);
	void maze_audio(machine_config &config);
	void shuffle_audio(machine_config &config);
	void tornbase_audio(machine_config &config);

	void blueshrk_io_map(address_map &map);
	void bowler_io_map(address_map &map);
	void checkmat_io_map(address_map &map);
	void dogpatch_io_map(address_map &map);
	void invad2ct_io_map(address_map &map);
	void invaders_io_map(address_map &map);
	void main_map(address_map &map);
	void maze_io_map(address_map &map);
	void phantom2_io_map(address_map &map);
	void shuffle_io_map(address_map &map);
	void tornbase_io_map(address_map &map);
};


#define SEAWOLF_ERASE_SW_PORT_TAG   ("ERASESW")
#define SEAWOLF_ERASE_DIP_PORT_TAG  ("ERASEDIP")

class seawolf_state : public mw8080bw_state
{
public:
	seawolf_state(machine_config const &mconfig, device_type type, char const *tag) :
		mw8080bw_state(mconfig, type, tag),
		m_erase_sw(*this, SEAWOLF_ERASE_SW_PORT_TAG),
		m_erase_dip(*this, SEAWOLF_ERASE_DIP_PORT_TAG),
		m_exp_lamps(*this, "EXP_LAMP_%X", 0U),
		m_torp_lamps(*this, "TORP_LAMP_%u", 1U),
		m_ready_lamp(*this, "READY_LAMP"),
		m_reload_lamp(*this, "RELOAD_LAMP")
	{
	}

	void seawolf(machine_config &config);

	DECLARE_CUSTOM_INPUT_MEMBER(erase_input_r);

protected:
	virtual void machine_start() override;

private:
	void explosion_lamp_w(u8 data);
	void periscope_lamp_w(u8 data);

	void io_map(address_map &map);

	required_ioport m_erase_sw;
	required_ioport m_erase_dip;
	output_finder<16> m_exp_lamps;
	output_finder<4> m_torp_lamps;
	output_finder<> m_ready_lamp;
	output_finder<> m_reload_lamp;
};


class gunfight_state : public mw8080bw_state
{
public:
	gunfight_state(machine_config const &mconfig, device_type type, char const *tag) :
		mw8080bw_state(mconfig, type, tag),
		m_soundboard(*this, "soundboard")
	{
	}

	void gunfight(machine_config &config);

private:
	void io_w(offs_t offset, u8 data);

	void io_map(address_map &map);

	required_device<gunfight_audio_device> m_soundboard;
};


class boothill_state : public mw8080bw_state
{
public:
	boothill_state(machine_config const &mconfig, device_type type, char const *tag) :
		mw8080bw_state(mconfig, type, tag)
	{
	}

	void boothill(machine_config &config);
	void gmissile(machine_config &config);
	void m4(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	u8 reversible_shift_result_r();
	void reversible_shift_count_w(u8 data);

	void boothill_io_map(address_map &map);
	void gmissile_io_map(address_map &map);
	void m4_io_map(address_map &map);

	u8 m_rev_shift_res;
};


#define DESERTGU_GUN_X_PORT_TAG         ("GUNX")
#define DESERTGU_GUN_Y_PORT_TAG         ("GUNY")

#define DESERTGU_DIP_SW_0_1_SET_1_TAG   ("DIPSW01SET1")
#define DESERTGU_DIP_SW_0_1_SET_2_TAG   ("DIPSW01SET2")

class desertgu_state : public mw8080bw_state
{
public:
	desertgu_state(machine_config const &mconfig, device_type type, char const *tag) :
		mw8080bw_state(mconfig, type, tag),
		m_gun_port(*this, { DESERTGU_GUN_Y_PORT_TAG, DESERTGU_GUN_X_PORT_TAG }),
		m_dip_sw_0_1(*this, { DESERTGU_DIP_SW_0_1_SET_1_TAG, DESERTGU_DIP_SW_0_1_SET_2_TAG })
	{
	}

	void desertgu(machine_config &config);

	DECLARE_CUSTOM_INPUT_MEMBER(gun_input_r);
	DECLARE_CUSTOM_INPUT_MEMBER(dip_sw_0_1_r);

protected:
	virtual void machine_start() override;

private:
	void io_map(address_map &map);

	required_ioport_array<2> m_gun_port;
	required_ioport_array<2> m_dip_sw_0_1;
	u8 m_controller_select;
};


#define DPLAY_L_PITCH_PORT_TAG      ("LPITCH")
#define DPLAY_R_PITCH_PORT_TAG      ("RPITCH")
#define DPLAY_CAB_TYPE_PORT_TAG     ("CAB")

class dplay_state : public mw8080bw_state
{
public:
	dplay_state(machine_config const &mconfig, device_type type, char const *tag) :
		mw8080bw_state(mconfig, type, tag),
		m_l_pitch(*this, DPLAY_L_PITCH_PORT_TAG),
		m_r_pitch(*this, DPLAY_L_PITCH_PORT_TAG),
		m_cab_type(*this, DPLAY_CAB_TYPE_PORT_TAG)
	{
	}

	void dplay(machine_config &config);

	DECLARE_CUSTOM_INPUT_MEMBER(dplay_pitch_left_input_r);
	DECLARE_CUSTOM_INPUT_MEMBER(dplay_pitch_right_input_r);

private:
	void io_map(address_map &map);

	required_ioport m_l_pitch;
	required_ioport m_r_pitch;
	required_ioport m_cab_type;
};


class clowns_state : public mw8080bw_state
{
public:
	clowns_state(machine_config const &mconfig, device_type type, char const *tag) :
		mw8080bw_state(mconfig, type, tag),
		m_controllers(*this, "CONTP%u", 1U)
	{
	}

	void clowns(machine_config &config);
	void spacwalk(machine_config &config);

	DECLARE_CUSTOM_INPUT_MEMBER(controller_r);

protected:
	virtual void machine_start() override;

private:
	void clowns_io_map(address_map &map);
	void spacwalk_io_map(address_map &map);

	required_ioport_array<2> m_controllers;
	u8 m_controller_select;
};


class spcenctr_state : public mw8080bw_state
{
public:
	spcenctr_state(machine_config const &mconfig, device_type type, char const *tag) :
		mw8080bw_state(mconfig, type, tag),
		m_soundboard(*this, "soundboard")
	{
	}

	void spcenctr(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	void io_w(offs_t offset, u8 data);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect);

	void io_map(address_map &map);

	required_device<spcenctr_audio_device> m_soundboard;
	u8 m_trench_width;
	u8 m_trench_center;
	u8 m_trench_slope[16];  // 16x4 bit RAM
	u8 m_bright_control;
	u8 m_brightness;
};


class zzzap_state : public mw8080bw_state
{
public:
	zzzap_state(machine_config const &mconfig, device_type type, char const *tag) :
		mw8080bw_state(mconfig, type, tag)
	{
	}

	void zzzap(machine_config &config);

	void lagunar(machine_config &config);

protected:
	void zzzap_common(machine_config &config);

private:
	void io_map(address_map &map);
};


#define TORNBASE_CAB_TYPE_UPRIGHT_OLD   (0)
#define TORNBASE_CAB_TYPE_UPRIGHT_NEW   (1)
#define TORNBASE_CAB_TYPE_COCKTAIL      (2)

#define INVADERS_CAB_TYPE_PORT_TAG      ("CAB")
#define INVADERS_P1_CONTROL_PORT_TAG    ("CONTP1")
#define INVADERS_P2_CONTROL_PORT_TAG    ("CONTP2")
#define INVADERS_SW6_SW7_PORT_TAG       ("SW6SW7")
#define INVADERS_SW5_PORT_TAG           ("SW5")

#define BLUESHRK_SPEAR_PORT_TAG         ("IN0")

#define INVADERS_CONTROL_PORT_P1 \
	PORT_START(INVADERS_P1_CONTROL_PORT_TAG) \
	INVADERS_CONTROL_PORT_PLAYER(1)

#define INVADERS_CONTROL_PORT_P2 \
	PORT_START(INVADERS_P2_CONTROL_PORT_TAG) \
	INVADERS_CONTROL_PORT_PLAYER(2)

#define INVADERS_CONTROL_PORT_PLAYER(player) \
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(player) \
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(player) \
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(player) \
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )

#define INVADERS_CAB_TYPE_PORT \
	PORT_START(INVADERS_CAB_TYPE_PORT_TAG) \
	PORT_CONFNAME( 0x01, 0x00, DEF_STR( Cabinet ) ) \
	PORT_CONFSETTING(    0x00, DEF_STR( Upright ) ) \
	PORT_CONFSETTING(    0x01, DEF_STR( Cocktail ) )

/*----------- defined in drivers/mw8080bw.c -----------*/

extern const internal_layout layout_invaders;

#endif // MAME_INCLUDES_MW8080BW_H
