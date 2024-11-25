// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer
/*****************************************************************************
 *
 * includes/vectrex.h
 *
 ****************************************************************************/
#ifndef MAME_MILTONBRADLEY_VECTREX_H
#define MAME_MILTONBRADLEY_VECTREX_H

#pragma once

#include "machine/6522via.h"
#include "sound/dac.h"
#include "sound/ay8910.h"
#include "video/vector.h"

#include "bus/vectrex/slot.h"
#include "bus/vectrex/rom.h"

#include "screen.h"

#define NVECT 10000

class vectrex_base_state : public driver_device
{
public:
	void vectrex_cart(device_slot_interface &device);

protected:
	vectrex_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cart(*this, "cartslot"),
		m_via6522_0(*this, "via6522_0"),
		m_gce_vectorram(*this, "gce_vectorram"),
		m_dac(*this, "dac"),
		m_ay8912(*this, "ay8912"),
		m_vector(*this, "vector"),
		m_io_contr(*this, {"CONTR1X", "CONTR1Y", "CONTR2X", "CONTR2Y"}),
		m_io_buttons(*this, "BUTTONS"),
		m_io_3dconf(*this, "3DCONF"),
		m_io_lpenconf(*this, "LPENCONF"),
		m_io_lpenx(*this, "LPENX"),
		m_io_lpeny(*this, "LPENY"),
		m_screen(*this, "screen")
	{ }

	void psg_port_w(uint8_t data);
	uint8_t via_r(offs_t offset);
	void via_w(offs_t offset, uint8_t data);
	virtual void driver_start() override;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(imager_change_color);
	TIMER_CALLBACK_MEMBER(update_level);
	TIMER_CALLBACK_MEMBER(imager_eye);
	TIMER_CALLBACK_MEMBER(imager_index);
	TIMER_CALLBACK_MEMBER(lightpen_trigger);
	TIMER_CALLBACK_MEMBER(refresh);
	TIMER_CALLBACK_MEMBER(zero_integrators);
	TIMER_CALLBACK_MEMBER(update_analog);
	TIMER_CALLBACK_MEMBER(update_blank);
	TIMER_CALLBACK_MEMBER(update_mux_enable);
	TIMER_CALLBACK_MEMBER(update_ramp);
	void update_vector();
	uint8_t via_pb_r();
	uint8_t via_pa_r();
	void via_pb_w(uint8_t data);
	void via_pa_w(uint8_t data);
	void via_ca2_w(int state);
	void via_cb2_w(int state);
	void via_irq(int state);

	void vectrex_base(machine_config &config);

	void configure_imager(bool reset_refresh, const double *imager_angles);
	void screen_configuration();
	void multiplexer(int mux);
	void add_point(int x, int y, rgb_t color, int intensity);
	void add_point_stereo(int x, int y, rgb_t color, int intensity);

	unsigned char m_via_out[2];

	required_device<cpu_device> m_maincpu;
	optional_device<vectrex_cart_slot_device> m_cart;

	double m_imager_freq = 0;
	emu_timer *m_imager_color_timers[3]{};
	emu_timer *m_imager_eye_timer = nullptr;
	emu_timer *m_imager_index_timer = nullptr;
	emu_timer *m_imager_level_timer = nullptr;
	emu_timer *m_lp_t = nullptr;

	required_device<via6522_device> m_via6522_0;

private:

	struct vectrex_point
	{
		int x = 0; int y = 0;
		rgb_t col;
		int intensity = 0;
	};

	required_shared_ptr<uint8_t> m_gce_vectorram;
	int m_imager_status = 0;
	uint32_t m_beam_color = 0;
	int m_lightpen_port = 0;
	int m_reset_refresh = 0;
	const double *m_imager_angles = nullptr;
	rgb_t m_imager_colors[6];
	unsigned char m_imager_pinlevel = 0;
	int m_old_mcontrol = 0;
	double m_sl = 0;
	double m_pwl = 0;
	int m_x_center = 0;
	int m_y_center = 0;
	int m_x_max = 0;
	int m_y_max = 0;
	int m_x_int = 0;
	int m_y_int = 0;
	int m_lightpen_down = 0;
	int m_pen_x = 0;
	int m_pen_y = 0;
	emu_timer *m_refresh = nullptr;
	emu_timer *m_zero_integrators_timer = nullptr;
	emu_timer *m_update_blank_timer = nullptr;
	emu_timer *m_update_mux_enable_timer = nullptr;
	uint8_t m_blank = 0;
	uint8_t m_ramp = 0;
	int8_t m_analog[5]{};
	int m_point_index = 0;
	int m_display_start = 0;
	int m_display_end = 0;
	vectrex_point m_points[NVECT];
	uint16_t m_via_timer2 = 0;
	attotime m_vector_start_time;
	uint8_t m_cb2 = 0;
	void (vectrex_base_state::*vector_add_point_function)(int, int, rgb_t, int);

	required_device<mc1408_device> m_dac;
	required_device<ay8910_device> m_ay8912;
	required_device<vector_device> m_vector;
	optional_ioport_array<4> m_io_contr;
	required_ioport m_io_buttons;
	required_ioport m_io_3dconf;
	required_ioport m_io_lpenconf;
	required_ioport m_io_lpenx;
	required_ioport m_io_lpeny;
	required_device<screen_device> m_screen;
};


class vectrex_state : public vectrex_base_state
{
public:
	vectrex_state(const machine_config &mconfig, device_type type, const char *tag) :
		vectrex_base_state(mconfig, type, tag)
	{ }

	void vectrex(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	void vectrex_map(address_map &map) ATTR_COLD;
};


class raaspec_state : public vectrex_base_state
{
public:
	raaspec_state(const machine_config &mconfig, device_type type, const char *tag) :
		vectrex_base_state(mconfig, type, tag),
		m_io_coin(*this, "COIN")
	{ }

	void raaspec(machine_config &config);

private:
	void raaspec_led_w(uint8_t data);
	uint8_t s1_via_pb_r();

	void raaspec_map(address_map &map) ATTR_COLD;

	required_ioport m_io_coin;
};

#endif // MAME_MILTONBRADLEY_VECTREX_H
