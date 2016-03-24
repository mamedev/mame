// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer
/*****************************************************************************
 *
 * includes/vectrex.h
 *
 ****************************************************************************/

#ifndef VECTREX_H_
#define VECTREX_H_

#include "machine/6522via.h"
#include "sound/dac.h"
#include "sound/ay8910.h"
#include "video/vector.h"

#include "bus/vectrex/slot.h"
#include "bus/vectrex/rom.h"

#define NVECT 10000

struct vectrex_point
{
	int x; int y;
	rgb_t col;
	int intensity;
};

class vectrex_state : public driver_device
{
public:
	enum
	{
		TIMER_VECTREX_IMAGER_CHANGE_COLOR,
		TIMER_UPDATE_LEVEL,
		TIMER_VECTREX_IMAGER_EYE,
		TIMER_LIGHTPEN_TRIGGER,
		TIMER_VECTREX_REFRESH,
		TIMER_VECTREX_ZERO_INTEGRATORS,
		TIMER_UPDATE_SIGNAL
	};

	vectrex_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_gce_vectorram(*this, "gce_vectorram"),
		m_maincpu(*this, "maincpu"),
		m_via6522_0(*this, "via6522_0"),
		m_dac(*this, "dac"),
		m_ay8912(*this, "ay8912"),
		m_vector(*this, "vector"),
		m_cart(*this, "cartslot"),
		m_io_contr1x(*this, "CONTR1X"),
		m_io_contr1y(*this, "CONTR1Y"),
		m_io_contr2x(*this, "CONTR2X"),
		m_io_contr2y(*this, "CONTR2Y"),
		m_io_buttons(*this, "BUTTONS"),
		m_io_3dconf(*this, "3DCONF"),
		m_io_lpenconf(*this, "LPENCONF"),
		m_io_lpenx(*this, "LPENX"),
		m_io_lpeny(*this, "LPENY"),
		m_io_coin(*this, "COIN"),
		m_screen(*this, "screen")
	{ }

	required_shared_ptr<UINT8> m_gce_vectorram;
	int m_imager_status;
	UINT32 m_beam_color;
	unsigned char m_via_out[2];
	double m_imager_freq;
	emu_timer *m_imager_timer;
	int m_lightpen_port;
	int m_reset_refresh;
	rgb_t m_imager_colors[6];
	const double *m_imager_angles;
	unsigned char m_imager_pinlevel;
	int m_old_mcontrol;
	double m_sl;
	double m_pwl;
	int m_x_center;
	int m_y_center;
	int m_x_max;
	int m_y_max;
	int m_x_int;
	int m_y_int;
	int m_lightpen_down;
	int m_pen_x;
	int m_pen_y;
	emu_timer *m_lp_t;
	emu_timer *m_refresh;
	UINT8 m_blank;
	UINT8 m_ramp;
	INT8 m_analog[5];
	int m_point_index;
	int m_display_start;
	int m_display_end;
	vectrex_point m_points[NVECT];
	UINT16 m_via_timer2;
	attotime m_vector_start_time;
	UINT8 m_cb2;
	void (vectrex_state::*vector_add_point_function)(int, int, rgb_t, int);
	DECLARE_WRITE8_MEMBER(vectrex_psg_port_w);
	DECLARE_READ8_MEMBER(vectrex_via_r);
	DECLARE_WRITE8_MEMBER(vectrex_via_w);
	DECLARE_WRITE8_MEMBER(raaspec_led_w);
	DECLARE_DRIVER_INIT(vectrex);
	virtual void video_start() override;
	virtual void machine_start() override;
	DECLARE_VIDEO_START(raaspec);
	UINT32 screen_update_vectrex(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(vectrex_imager_change_color);
	TIMER_CALLBACK_MEMBER(update_level);
	TIMER_CALLBACK_MEMBER(vectrex_imager_eye);
	TIMER_CALLBACK_MEMBER(lightpen_trigger);
	TIMER_CALLBACK_MEMBER(vectrex_refresh);
	TIMER_CALLBACK_MEMBER(vectrex_zero_integrators);
	TIMER_CALLBACK_MEMBER(update_signal);
	DECLARE_READ8_MEMBER(vectrex_via_pb_r);
	DECLARE_READ8_MEMBER(vectrex_via_pa_r);
	DECLARE_READ8_MEMBER(vectrex_s1_via_pb_r);
	DECLARE_WRITE8_MEMBER(v_via_pb_w);
	DECLARE_WRITE8_MEMBER(v_via_pa_w);
	DECLARE_WRITE_LINE_MEMBER(v_via_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(v_via_cb2_w);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( vectrex_cart );
	DECLARE_WRITE_LINE_MEMBER(vectrex_via_irq);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	required_device<cpu_device> m_maincpu;
	required_device<via6522_device> m_via6522_0;
	required_device<dac_device> m_dac;
	required_device<ay8910_device> m_ay8912;
	required_device<vector_device> m_vector;
	optional_device<vectrex_cart_slot_device> m_cart;
	optional_ioport m_io_contr1x;
	optional_ioport m_io_contr1y;
	optional_ioport m_io_contr2x;
	optional_ioport m_io_contr2y;
	required_ioport m_io_buttons;
	required_ioport m_io_3dconf;
	required_ioport m_io_lpenconf;
	required_ioport m_io_lpenx;
	required_ioport m_io_lpeny;
	optional_ioport m_io_coin;
	required_device<screen_device> m_screen;

	void vectrex_configuration();
	void vectrex_multiplexer(int mux);
	void vectrex_add_point(int x, int y, rgb_t color, int intensity);
	void vectrex_add_point_stereo(int x, int y, rgb_t color, int intensity);
};

#endif /* VECTREX_H_ */
