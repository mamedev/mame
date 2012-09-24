/*****************************************************************************
 *
 * includes/vectrex.h
 *
 ****************************************************************************/

#ifndef VECTREX_H_
#define VECTREX_H_

#include "machine/6522via.h"

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
	vectrex_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_gce_vectorram(*this, "gce_vectorram"){ }

	required_shared_ptr<UINT8> m_gce_vectorram;
	int m_64k_cart;
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
	void (*vector_add_point_function) (running_machine &, int, int, rgb_t, int);
	DECLARE_WRITE8_MEMBER(vectrex_psg_port_w);
	DECLARE_READ8_MEMBER(vectrex_via_r);
	DECLARE_WRITE8_MEMBER(vectrex_via_w);
	DECLARE_WRITE8_MEMBER(raaspec_led_w);
	DECLARE_DRIVER_INIT(vectrex);
	virtual void video_start();
	DECLARE_VIDEO_START(raaspec);
	UINT32 screen_update_vectrex(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(vectrex_imager_change_color);
	TIMER_CALLBACK_MEMBER(update_level);
	TIMER_CALLBACK_MEMBER(vectrex_imager_eye);
	TIMER_CALLBACK_MEMBER(lightpen_trigger);
	TIMER_CALLBACK_MEMBER(vectrex_refresh);
	TIMER_CALLBACK_MEMBER(vectrex_zero_integrators);
	TIMER_CALLBACK_MEMBER(update_signal);
};


/*----------- defined in machine/vectrex.c -----------*/

DEVICE_IMAGE_LOAD( vectrex_cart );
TIMER_CALLBACK(vectrex_imager_eye);
void vectrex_configuration(running_machine &machine);
DECLARE_READ8_DEVICE_HANDLER (vectrex_via_pa_r);
DECLARE_READ8_DEVICE_HANDLER(vectrex_via_pb_r );
void vectrex_via_irq (device_t *device, int level);

/* for spectrum 1+ */
DECLARE_READ8_DEVICE_HANDLER( vectrex_s1_via_pb_r );

/*----------- defined in video/vectrex.c -----------*/

extern const via6522_interface vectrex_via6522_interface;
extern const via6522_interface spectrum1_via6522_interface;

void vectrex_add_point_stereo (running_machine &machine, int x, int y, rgb_t color, int intensity);
void vectrex_add_point (running_machine &machine, int x, int y, rgb_t color, int intensity);

#endif /* VECTREX_H_ */
