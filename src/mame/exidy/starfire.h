// license:BSD-3-Clause
// copyright-holders:Dan Boris, Olivier Galibert, Aaron Giles
/***************************************************************************

    Star Fire/Fire One system

***************************************************************************/
#ifndef MAME_EXIDY_STARFIRE_H
#define MAME_EXIDY_STARFIRE_H

#pragma once

#include "machine/netlist.h"
#include "machine/pit8253.h"
#include "netlist/nl_setup.h"
#include "nl_fireone.h"
#include "nl_starfire.h"
#include "sound/dac.h"
#include "screen.h"


#define STARFIRE_MASTER_CLOCK   (XTAL(20'000'000))
#define STARFIRE_CPU_CLOCK      (STARFIRE_MASTER_CLOCK / 8)
#define STARFIRE_PIXEL_CLOCK    (STARFIRE_MASTER_CLOCK / 4)
#define STARFIRE_HTOTAL         (0x13f)  /* could be 0x140, but I think this is right */
#define STARFIRE_HBEND          (0x000)
#define STARFIRE_HBSTART        (0x100)
#define STARFIRE_VTOTAL         (0x106)
#define STARFIRE_VBEND          (0x020)
#define STARFIRE_VBSTART        (0x100)
#define STARFIRE_NUM_PENS       (0x40)


class starfire_base_state : public driver_device
{
public:
	starfire_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_colorram(*this, "colorram")
		, m_videoram(*this, "videoram")
		, m_dsw(*this, "DSW")
		, m_system(*this, "SYSTEM")
	{ }

protected:
	virtual void video_start() override ATTR_COLD;

	void base_config(machine_config &config);
	void main_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;
	required_ioport m_dsw;
	required_ioport m_system;

	uint8_t m_vidctrl = 0;
	uint8_t m_vidctrl1 = 0;
	uint8_t m_color = 0;
	uint16_t m_colors[STARFIRE_NUM_PENS] = { };

	emu_timer* m_scanline_timer;
	bitmap_rgb32 m_screen_bitmap;

	virtual uint8_t input_r(offs_t offset) = 0;
	virtual void sound_w(offs_t offset, uint8_t data) = 0;
	virtual void music_w(offs_t offset, uint8_t data) { }

	void scratch_w(offs_t offset, uint8_t data);
	uint8_t scratch_r(offs_t offset);
	void colorram_w(offs_t offset, uint8_t data);
	uint8_t colorram_r(offs_t offset);
	void videoram_w(offs_t offset, uint8_t data);
	uint8_t videoram_r(offs_t offset);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(scanline_callback);
	void get_pens(pen_t *pens);
};

class starfire_state : public starfire_base_state
{
public:
	starfire_state(const machine_config &mconfig, device_type type, const char *tag)
		: starfire_base_state(mconfig, type, tag)
		, m_dac(*this, "dac") // just to have a sound device
		, m_nmi(*this, "NMI")
		, m_stickz(*this, "STICKZ")
		, m_stickx(*this, "STICKX")
		, m_sticky(*this, "STICKY")
		, m_sound_size(*this, "sound_nl:size")
		, m_sound_explosion(*this, "sound_nl:sexplo")
		, m_sound_tie(*this, "sound_nl:stie")
		, m_sound_laser(*this, "sound_nl:slaser")
		, m_sound_track(*this, "sound_nl:track")
		, m_sound_lock(*this, "sound_nl:lock")
		, m_sound_scanner(*this, "sound_nl:scanner")
		, m_sound_overheat(*this, "sound_nl:oheat")
	{ }

	void starfire(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<dac_word_interface> m_dac;

	required_ioport m_nmi;
	required_ioport m_stickz;
	required_ioport m_stickx;
	required_ioport m_sticky;

	virtual uint8_t input_r(offs_t offset) override;
	virtual void sound_w(offs_t offset, uint8_t data) override;

	INTERRUPT_GEN_MEMBER(vblank_int);

	NETDEV_ANALOG_CALLBACK_MEMBER(tieon1_cb);
	NETDEV_ANALOG_CALLBACK_MEMBER(laseron1_cb);
	NETDEV_ANALOG_CALLBACK_MEMBER(sound_out_cb);

	uint8_t m_sound_tie_on = 0;
	uint8_t m_sound_laser_on = 0;

	required_device<netlist_mame_logic_input_device> m_sound_size;
	required_device<netlist_mame_logic_input_device> m_sound_explosion;
	required_device<netlist_mame_logic_input_device> m_sound_tie;
	required_device<netlist_mame_logic_input_device> m_sound_laser;
	required_device<netlist_mame_logic_input_device> m_sound_track;
	required_device<netlist_mame_logic_input_device> m_sound_lock;
	required_device<netlist_mame_logic_input_device> m_sound_scanner;
	required_device<netlist_mame_logic_input_device> m_sound_overheat;
};

class fireone_state : public starfire_base_state
{
public:
	fireone_state(const machine_config &mconfig, device_type type, const char *tag)
		: starfire_base_state(mconfig, type, tag)
		, m_pit(*this, "pit")
		, m_controls(*this, "P%u", 1U)
		, m_sound_left_partial_hit(*this, "sound_nl:lshpht")
		, m_sound_right_partial_hit(*this, "sound_nl:rshpht")
		, m_sound_left_torpedo(*this, "sound_nl:ltorp")
		, m_sound_right_torpedo(*this, "sound_nl:rtorp")
		, m_sound_left_boom(*this, "sound_nl:lboom")
		, m_sound_right_boom(*this, "sound_nl:rboom")
		, m_sound_torpedo_collision(*this, "sound_nl:torpcoll")
		, m_sound_submarine_engine(*this, "sound_nl:subeng")
		, m_sound_alert(*this, "sound_nl:alert")
		, m_sound_sonar_enable(*this, "sound_nl:sonar_enable")
		, m_sound_sonar_sync(*this, "sound_nl:sonar_sync")
		, m_sound_off_left(*this, "sound_nl:lsound_off")
		, m_sound_off_right(*this, "sound_nl:rsound_off")
		, m_music_a(*this, "sound_nl:music_a")
		, m_music_b(*this, "sound_nl:music_b")
		, m_music_c(*this, "sound_nl:music_c")
	{ }

	void fireone(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<pit8253_device> m_pit;
	required_ioport_array<2> m_controls;

	void music_a_out_cb(int state);
	void music_b_out_cb(int state);
	void music_c_out_cb(int state);

	virtual uint8_t input_r(offs_t offset) override;
	virtual void sound_w(offs_t offset, uint8_t data) override;
	virtual void music_w(offs_t offset, uint8_t data) override;

	uint8_t m_player_select = 0;

	INTERRUPT_GEN_MEMBER(vblank_int);

	required_device<netlist_mame_logic_input_device> m_sound_left_partial_hit;
	required_device<netlist_mame_logic_input_device> m_sound_right_partial_hit;
	required_device<netlist_mame_logic_input_device> m_sound_left_torpedo;
	required_device<netlist_mame_logic_input_device> m_sound_right_torpedo;
	required_device<netlist_mame_logic_input_device> m_sound_left_boom;
	required_device<netlist_mame_logic_input_device> m_sound_right_boom;
	required_device<netlist_mame_logic_input_device> m_sound_torpedo_collision;
	required_device<netlist_mame_logic_input_device> m_sound_submarine_engine;
	required_device<netlist_mame_logic_input_device> m_sound_alert;
	required_device<netlist_mame_logic_input_device> m_sound_sonar_enable;
	required_device<netlist_mame_logic_input_device> m_sound_sonar_sync;
	required_device<netlist_mame_logic_input_device> m_sound_off_left;
	required_device<netlist_mame_logic_input_device> m_sound_off_right;
	required_device<netlist_mame_logic_input_device> m_music_a;
	required_device<netlist_mame_logic_input_device> m_music_b;
	required_device<netlist_mame_logic_input_device> m_music_c;
};

#endif // MAME_EXIDY_STARFIRE_H
