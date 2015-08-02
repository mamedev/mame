// license:GPL-2.0+
// copyright-holders:Raphael Nabet
/*****************************************************************************
 *
 * includes/tx0.h
 *
 ****************************************************************************/

#ifndef TX0_H_
#define TX0_H_

#include "video/crt.h"
#include "cpu/pdp1/tx0.h"

enum state_t
{
	MTS_UNSELECTED,
	MTS_SELECTING,
	MTS_SELECTED,
	MTS_UNSELECTING
};

enum backspace_state_t
{
	MTBSS_STATE0,
	MTBSS_STATE1,
	MTBSS_STATE2,
	MTBSS_STATE3,
	MTBSS_STATE4,
	MTBSS_STATE5,
	MTBSS_STATE6
};

enum state_2_t
{
	MTRDS_STATE0,
	MTRDS_STATE1,
	MTRDS_STATE2,
	MTRDS_STATE3,
	MTRDS_STATE4,
	MTRDS_STATE5,
	MTRDS_STATE6
};

enum state_3_t
{
	MTWTS_STATE0,
	MTWTS_STATE1,
	MTWTS_STATE2,
	MTWTS_STATE3
};

enum irg_pos_t
{
	MTIRGP_START,
	MTIRGP_ENDMINUS1,
	MTIRGP_END
};



/* tape reader registers */
struct tx0_tape_reader_t
{
	device_image_interface *fd; /* file descriptor of tape image */

	int motor_on;   /* 1-bit reader motor on */

	int rcl;        /* 1-bit reader clutch */
	int rc;         /* 2-bit reader counter */

	emu_timer *timer;   /* timer to simulate reader timing */
};



/* tape puncher registers */
struct tape_puncher_t
{
	device_image_interface *fd; /* file descriptor of tape image */

	emu_timer *timer;   /* timer to generate completion pulses */
};



/* typewriter registers */
struct tx0_typewriter_t
{
	device_image_interface *fd; /* file descriptor of output image */

	emu_timer *prt_timer;/* timer to generate completion pulses */
};


/* magnetic tape unit registers */
struct magtape_t
{
	device_image_interface *img;        /* image descriptor */

	state_t state;

	int command;
	int binary_flag;

	union
	{
		backspace_state_t backspace_state;
		struct
		{
			state_2_t state;
			int space_flag;
		} read;
		struct
		{
			state_3_t state;
			int counter;
		} write;
	} u;

	int sel_pending;
	int cpy_pending;

	irg_pos_t irg_pos;          /* position relative to inter-record gap */

	int long_parity;

	emu_timer *timer;   /* timer to simulate reader timing */
};


class tx0_state : public driver_device
{
public:
	tx0_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_crt(*this, "crt"),
		m_csw(*this, "CSW"),
		m_twr(*this, "TWR")
	{ }

	tx0_tape_reader_t m_tape_reader;
	tape_puncher_t m_tape_puncher;
	tx0_typewriter_t m_typewriter;
	emu_timer *m_dis_timer;
	magtape_t m_magtape;
	int m_old_typewriter_keys[4];
	int m_old_control_keys;
	int m_old_tsr_keys;
	int m_tsr_index;
	int m_typewriter_color;
	bitmap_ind16 m_panel_bitmap;
	bitmap_ind16 m_typewriter_bitmap;
	int m_pos;
	int m_case_shift;
	DECLARE_DRIVER_INIT(tx0);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_PALETTE_INIT(tx0);
	UINT32 screen_update_tx0(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_tx0(screen_device &screen, bool state);
	INTERRUPT_GEN_MEMBER(tx0_interrupt);
	TIMER_CALLBACK_MEMBER(reader_callback);
	TIMER_CALLBACK_MEMBER(puncher_callback);
	TIMER_CALLBACK_MEMBER(prt_callback);
	TIMER_CALLBACK_MEMBER(dis_callback);
	void tx0_machine_stop();
	required_device<tx0_device> m_maincpu;
	inline void tx0_plot_pixel(bitmap_ind16 &bitmap, int x, int y, UINT32 color);
	void tx0_plot(int x, int y);
	void tx0_draw_led(bitmap_ind16 &bitmap, int x, int y, int state);
	void tx0_draw_multipleled(bitmap_ind16 &bitmap, int x, int y, int value, int nb_bits);
	void tx0_draw_switch(bitmap_ind16 &bitmap, int x, int y, int state);
	void tx0_draw_multipleswitch(bitmap_ind16 &bitmap, int x, int y, int value, int nb_bits);
	void tx0_draw_char(bitmap_ind16 &bitmap, char character, int x, int y, int color);
	void tx0_draw_string(bitmap_ind16 &bitmap, const char *buf, int x, int y, int color);
	void tx0_draw_vline(bitmap_ind16 &bitmap, int x, int y, int height, int color);
	void tx0_draw_hline(bitmap_ind16 &bitmap, int x, int y, int width, int color);
	void tx0_draw_panel_backdrop(bitmap_ind16 &bitmap);
	void tx0_draw_panel(bitmap_ind16 &bitmap);
	void tx0_typewriter_linefeed();
	void tx0_typewriter_drawchar(int character);
	int tape_read(UINT8 *reply);
	void tape_write(UINT8 data);
	void begin_tape_read(int binary);
	void typewriter_out(UINT8 data);
	void schedule_select();
	void schedule_unselect();
	void tx0_keyboard();
	DECLARE_WRITE_LINE_MEMBER(tx0_io_cpy);
	DECLARE_WRITE_LINE_MEMBER(tx0_io_r1l);
	DECLARE_WRITE_LINE_MEMBER(tx0_io_r3l);
	DECLARE_WRITE_LINE_MEMBER(tx0_io_p6h);
	DECLARE_WRITE_LINE_MEMBER(tx0_io_p7h);
	DECLARE_WRITE_LINE_MEMBER(tx0_io_prt);
	DECLARE_WRITE_LINE_MEMBER(tx0_io_dis);
	DECLARE_WRITE_LINE_MEMBER(tx0_sel);
	DECLARE_WRITE_LINE_MEMBER(tx0_io_reset_callback);
	void magtape_callback();

private:
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<crt_device> m_crt;
	required_ioport m_csw;
	required_ioport_array<4> m_twr;
};

/* defines for each bit and mask in input port "CSW" */
enum
{
	/* bit numbers */
	tx0_control_bit     = 0,

	tx0_stop_c0_bit     = 1,
	tx0_stop_c1_bit     = 2,
	tx0_gbl_cm_sel_bit  = 3,
	tx0_stop_bit        = 4,
	tx0_restart_bit     = 5,
	tx0_read_in_bit     = 6,

	tx0_toggle_dn_bit   = 12,
	tx0_toggle_up_bit   = 13,
	tx0_cm_sel_bit      = 14,
	tx0_lr_sel_bit      = 15,

	/* masks */
	tx0_control = (1 << tx0_control_bit),

	tx0_stop_cyc0 = (1 << tx0_stop_c0_bit),
	tx0_stop_cyc1 = (1 << tx0_stop_c1_bit),
	tx0_gbl_cm_sel = (1 << tx0_gbl_cm_sel_bit),
	tx0_stop = (1 << tx0_stop_bit),
	tx0_restart = (1 << tx0_restart_bit),
	tx0_read_in = (1 << tx0_read_in_bit),

	tx0_toggle_dn = (1 << tx0_toggle_dn_bit),
	tx0_toggle_up = (1 << tx0_toggle_up_bit),
	tx0_cm_sel = (1 << tx0_cm_sel_bit),
	tx0_lr_sel = (1 << tx0_lr_sel_bit)
};

/* defines for our font */
enum
{
	tx0_charnum = /*96*/128,    /* ASCII set + xx special characters */
									/* for whatever reason, 96 breaks some characters */

	tx0_fontdata_size = 8 * tx0_charnum
};

enum
{
	/* size and position of crt window */
	crt_window_width = /*511*/512,
	crt_window_height = /*511*/512,
	crt_window_offset_x = 0,
	crt_window_offset_y = 0,
	/* size and position of operator control panel window */
	panel_window_width = 272,
	panel_window_height = 264,
	panel_window_offset_x = crt_window_width,
	panel_window_offset_y = 0,
	/* size and position of typewriter window */
	typewriter_window_width = 640,
	typewriter_window_height = 160,
	typewriter_window_offset_x = 0,
	typewriter_window_offset_y = crt_window_height
};

enum
{
	total_width = crt_window_width + panel_window_width,
	total_height = crt_window_height + typewriter_window_height,

	/* respect 4:3 aspect ratio to keep pixels square */
	virtual_width_1 = ((total_width+3)/4)*4,
	virtual_height_1 = ((total_height+2)/3)*3,
	virtual_width_2 = virtual_height_1*4/3,
	virtual_height_2 = virtual_width_1*3/4,
	virtual_width = (virtual_width_1 > virtual_width_2) ? virtual_width_1 : virtual_width_2,
	virtual_height = (virtual_height_1 > virtual_height_2) ? virtual_height_1 : virtual_height_2
};

enum
{   /* refresh rate in Hz: can be changed at will */
	refresh_rate = 60
};

/* Color codes */
enum
{
	/* first pen_crt_num_levels colors used for CRT (with remanence) */
	pen_crt_num_levels = 69,
	pen_crt_max_intensity = pen_crt_num_levels-1,

	/* next colors used for control panel and typewriter */
	pen_black = pen_crt_num_levels,
	pen_white,
	pen_green,
	pen_dk_green,
	pen_red,
	pen_lt_gray,

	/* color constants for control panel */
	pen_panel_bg = pen_black,
	pen_panel_caption = pen_white,
	color_panel_caption = 0,
	pen_switch_nut = pen_lt_gray,
	pen_switch_button = pen_white,
	pen_lit_lamp = pen_green,
	pen_unlit_lamp = pen_dk_green,

	/* color constants for typewriter */
	pen_typewriter_bg = pen_white,
	color_typewriter_black = 1,     /* palette 1 = black on white */
	color_typewriter_red = 2,       /* palette 2 = red on white */

	/* color constants used for light pen */
	pen_lightpen_nonpressed = pen_red,
	pen_lightpen_pressed = pen_green
};


#endif /* TX0_H_ */
