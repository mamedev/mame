// license:GPL-2.0+
// copyright-holders:Raphael Nabet
/*****************************************************************************
 *
 * includes/pdp1.h
 *
 ****************************************************************************/
#ifndef MAME_PDP1_PDP1_H
#define MAME_PDP1_PDP1_H

#pragma once

#include "mitcrt.h"

#include "cpu/pdp1/pdp1.h"
#include "imagedev/papertape.h"
#include "emupal.h"

/* defines for each bit and mask in input port "CSW" */
enum
{
	/* bit numbers */
	pdp1_control_bit = 0,

	pdp1_extend_bit     = 1,
	pdp1_start_nobrk_bit= 2,
	pdp1_start_brk_bit  = 3,
	pdp1_stop_bit       = 4,
	pdp1_continue_bit   = 5,
	pdp1_examine_bit    = 6,
	pdp1_deposit_bit    = 7,
	pdp1_read_in_bit    = 8,
	pdp1_reader_bit     = 9,
	pdp1_tape_feed_bit  = 10,
	pdp1_single_step_bit= 11,
	pdp1_single_inst_bit= 12,

	/* masks */
	pdp1_control = (1 << pdp1_control_bit),
	pdp1_extend = (1 << pdp1_extend_bit),
	pdp1_start_nobrk = (1 << pdp1_start_nobrk_bit),
	pdp1_start_brk = (1 << pdp1_start_brk_bit),
	pdp1_stop = (1 << pdp1_stop_bit),
	pdp1_continue = (1 << pdp1_continue_bit),
	pdp1_examine = (1 << pdp1_examine_bit),
	pdp1_deposit = (1 << pdp1_deposit_bit),
	pdp1_read_in = (1 << pdp1_read_in_bit),
	pdp1_reader = (1 << pdp1_reader_bit),
	pdp1_tape_feed = (1 << pdp1_tape_feed_bit),
	pdp1_single_step = (1 << pdp1_single_step_bit),
	pdp1_single_inst = (1 << pdp1_single_inst_bit)
};

/* defines for each bit in input port pdp1_spacewar_controllers*/
#define ROTATE_LEFT_PLAYER1       0x01
#define ROTATE_RIGHT_PLAYER1      0x02
#define THRUST_PLAYER1            0x04
#define FIRE_PLAYER1              0x08
#define ROTATE_LEFT_PLAYER2       0x10
#define ROTATE_RIGHT_PLAYER2      0x20
#define THRUST_PLAYER2            0x40
#define FIRE_PLAYER2              0x80
#define HSPACE_PLAYER1            0x100
#define HSPACE_PLAYER2            0x200

/* defines for each field in input port pdp1_config */
enum
{
	pdp1_config_extend_bit          = 0,
	pdp1_config_extend_mask         = 0x3,  /* 2 bits */
	pdp1_config_hw_mul_div_bit      = 2,
	pdp1_config_hw_mul_div_mask     = 0x1,
	/*pdp1_config_hw_obsolete_bit   = 3,
	pdp1_config_hw_obsolete_mask    = 0x1,*/
	pdp1_config_type_20_sbs_bit     = 4,
	pdp1_config_type_20_sbs_mask    = 0x1,
	pdp1_config_lightpen_bit        = 5,
	pdp1_config_lightpen_mask       = 0x1
};

/* defines for each field in input port pdp1_lightpen_state */
enum
{
	pdp1_lightpen_down_bit          = 0,
	pdp1_lightpen_smaller_bit       = 1,
	pdp1_lightpen_larger_bit        = 2,

	pdp1_lightpen_down              = (1 << pdp1_lightpen_down_bit),
	pdp1_lightpen_smaller           = (1 << pdp1_lightpen_smaller_bit),
	pdp1_lightpen_larger            = (1 << pdp1_lightpen_larger_bit)
};

/* defines for our font */
enum
{
	pdp1_charnum = /*104*/128,  /* ASCII set + 8 special characters */
									/* for whatever reason, 104 breaks some characters */

	pdp1_fontdata_size = 8 * pdp1_charnum
};

enum
{
	/* size and position of crt window */
	crt_window_width = 512,
	crt_window_height = 512,
	crt_window_offset_x = 0,
	crt_window_offset_y = 0,
	/* size and position of operator control panel window */
	panel_window_width = 384,
	panel_window_height = 128,
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
	color_typewriter_black = 1,
	color_typewriter_red = 2,

	/* color constants used for light pen */
	pen_lightpen_nonpressed = pen_red,
	pen_lightpen_pressed = pen_green
};

class pdp1_state;

// tape reader device
class pdp1_readtape_image_device :  public paper_tape_reader_device
{
public:
	// construction/destruction
	pdp1_readtape_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0U);
	virtual const char *image_interface() const noexcept override { return "pdp1_ptp"; }

	auto st_ptr() { return m_st_ptr.bind(); }

	void iot_rpa(int op2, int nac, int mb, int &io, int ac);
	void iot_rpb(int op2, int nac, int mb, int &io, int ac);
	void iot_rrb(int op2, int nac, int mb, int &io, int ac);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_image_interface implementation
	virtual const char *file_extensions() const noexcept override { return "tap,rim"; }

	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

public:
	TIMER_CALLBACK_MEMBER(reader_callback);

	int tape_read(uint8_t *reply);
	void begin_tape_read(int binary, int nac);

	required_device<pdp1_device> m_maincpu;

	devcb_write_line m_st_ptr;

	int m_motor_on; // 1-bit reader motor on

	int m_rb;       // 18-bit reader buffer
	int m_rcl;      // 1-bit reader clutch
	int m_rc;       // 2-bit reader counter
	int m_rby;      // 1-bit reader binary mode flip-flop
	int m_rcp;      // 1-bit reader "need a completion pulse" flip-flop

	emu_timer *m_timer;     // timer to simulate reader timing
};



// tape puncher device
class pdp1_punchtape_image_device : public paper_tape_punch_device
{
public:
	// construction/destruction
	pdp1_punchtape_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0U);

	auto st_ptp() { return m_st_ptp.bind(); }

	void iot_ppa(int op2, int nac, int mb, int &io, int ac);
	void iot_ppb(int op2, int nac, int mb, int &io, int ac);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_image_interface implementation
	virtual bool is_readable()  const noexcept override { return false; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return true; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *file_extensions() const noexcept override { return "tap,rim"; }

	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

public:
	TIMER_CALLBACK_MEMBER(puncher_callback);

	void tape_write(uint8_t data);

	required_device<pdp1_device> m_maincpu;

	devcb_write_line m_st_ptp;

	emu_timer *m_timer;     // timer to generate completion pulses
};



// typewriter device
class pdp1_typewriter_device :   public device_t,
									public device_image_interface
{
public:
	// construction/destruction
	pdp1_typewriter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0U);

	auto st_tyo() { return m_st_tyo.bind(); }
	auto st_tyi() { return m_st_tyi.bind(); }

	void iot_tyo(int op2, int nac, int mb, int &io, int ac);
	void iot_tyi(int op2, int nac, int mb, int &io, int ac);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_image_interface implementation
	virtual bool is_readable()  const noexcept override { return false; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return true; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual bool support_command_line_image_creation() const noexcept override { return true; }
	virtual const char *file_extensions() const noexcept override { return "typ"; }
	virtual const char *image_type_name() const noexcept override { return "printout"; }
	virtual const char *image_brief_type_name() const noexcept override { return "prin"; }

	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

public:
	TIMER_CALLBACK_MEMBER(tyo_callback);

	void linefeed();
	void drawchar(int character);
	void typewriter_out(uint8_t data);

	void pdp1_keyboard();

	required_device<pdp1_device> m_maincpu;
	required_device<pdp1_state> m_driver_state;
	required_ioport_array<4> m_twr;

	devcb_write_line m_st_tyo;
	devcb_write_line m_st_tyi;

	int m_tb;       // typewriter buffer

	emu_timer *m_tyo_timer; // timer to generate completion pulses

	int m_old_typewriter_keys[4];
	int m_color;
	bitmap_ind16 m_bitmap;
	int m_pos;
	int m_case_shift;
};

// MIT parallel drum (mostly similar to type 23)
class pdp1_cylinder_image_device :  public device_t,
									public device_image_interface
{
public:
	// construction/destruction
	pdp1_cylinder_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0U);

	void iot_dia(int op2, int nac, int mb, int &io, int ac);
	void iot_dba(int op2, int nac, int mb, int &io, int ac);
	void iot_dcc(int op2, int nac, int mb, int &io, int ac);
	void iot_dra(int op2, int nac, int mb, int &io, int ac);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_image_interface implementation
	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return true; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *file_extensions() const noexcept override { return "drm"; }
	virtual const char *image_type_name() const noexcept override { return "cylinder"; }
	virtual const char *image_brief_type_name() const noexcept override { return "cyln"; }

	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

public:
	void set_il(int il);
	TIMER_CALLBACK_MEMBER(rotation_timer_callback);
	TIMER_CALLBACK_MEMBER(il_timer_callback);
	[[maybe_unused]] void parallel_drum_init();
	uint32_t drum_read(int field, int position);
	void drum_write(int field, int position, uint32_t data);

	required_device<pdp1_device> m_maincpu;

	int m_il = 0;       // initial location (12-bit)
	int m_wc = 0;       // word counter (12-bit)
	int m_wcl = 0;      // word core location counter (16-bit)
	int m_rfb = 0;      // read field buffer (5-bit)
	int m_wfb = 0;      // write field buffer (5-bit)

	int m_dba = 0;

	emu_timer *m_rotation_timer = nullptr;// timer called each time dc is 0
	emu_timer *m_il_timer = nullptr;      // timer called each time dc is il
};


struct lightpen_t
{
	char active;
	char down;
	short x, y;
	short radius;
};


class pdp1_state : public driver_device
{
public:
	pdp1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_tape_reader(*this, "readt"),
		m_tape_puncher(*this, "punch"),
		m_typewriter(*this, "typewriter"),
		m_parallel_drum(*this, "drum"),
		m_dpy_timer(nullptr),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_crt(*this, "crt"),
		m_spacewar(*this, "SPACEWAR"),
		m_csw(*this, "CSW"),
		m_sense(*this, "SENSE"),
		m_tstadd(*this, "TSTADD"),
		m_twdmsb(*this, "TWDMSB"),
		m_twdlsb(*this, "TWDLSB"),
		m_cfg(*this, "CFG"),
		m_io_lightpen(*this, "LIGHTPEN"),
		m_lightx(*this, "LIGHTX"),
		m_lighty(*this, "LIGHTY")
	{ }

	required_device<pdp1_device> m_maincpu;
	required_device<pdp1_readtape_image_device> m_tape_reader;
	required_device<pdp1_punchtape_image_device> m_tape_puncher;
	required_device<pdp1_typewriter_device> m_typewriter;
	required_device<pdp1_cylinder_image_device> m_parallel_drum;
	int m_io_status;
	emu_timer *m_dpy_timer;
	lightpen_t m_lightpen;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	void pdp1_palette(palette_device &palette) const;
	uint32_t screen_update_pdp1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank_pdp1(int state);
	INTERRUPT_GEN_MEMBER(pdp1_interrupt);
	TIMER_CALLBACK_MEMBER(dpy_callback);
	void pdp1_machine_stop();
	inline void pdp1_plot_pixel(bitmap_ind16 &bitmap, int x, int y, uint32_t color);
	void pdp1_plot(int x, int y);
	void pdp1_draw_led(bitmap_ind16 &bitmap, int x, int y, int state);
	void pdp1_draw_multipleled(bitmap_ind16 &bitmap, int x, int y, int value, int nb_bits);
	void pdp1_draw_switch(bitmap_ind16 &bitmap, int x, int y, int state);
	void pdp1_draw_multipleswitch(bitmap_ind16 &bitmap, int x, int y, int value, int nb_bits);
	void pdp1_draw_char(bitmap_ind16 &bitmap, char character, int x, int y, int color);
	void pdp1_draw_string(bitmap_ind16 &bitmap, const char *buf, int x, int y, int color);
	void pdp1_draw_panel_backdrop(bitmap_ind16 &bitmap);
	void pdp1_draw_panel(bitmap_ind16 &bitmap);
	void pdp1_update_lightpen_state(const lightpen_t *new_state);
	void pdp1_draw_circle(bitmap_ind16 &bitmap, int x, int y, int radius, int color_);
	void pdp1_erase_lightpen(bitmap_ind16 &bitmap);
	void pdp1_draw_lightpen(bitmap_ind16 &bitmap);
	void pdp1_lightpen();

	template <int Mask> void io_status_w(int state);

	void pdp1(machine_config &config);
	void pdp1_map(address_map &map) ATTR_COLD;
private:
	void iot_dpy(int op2, int nac, int mb, int &io, int ac);

	void iot_011(int op2, int nac, int mb, int &io, int ac);

	void iot_cks(int op2, int nac, int mb, int &io, int ac);

	void io_start_clear();

	pdp1_reset_param_t m_reset_param;
	int m_old_lightpen;
	int m_old_control_keys;
	int m_old_tw_keys;
	int m_old_ta_keys;
	bitmap_ind16 m_panel_bitmap;
	lightpen_t m_lightpen_state;
	lightpen_t m_previous_lightpen_state;

public:
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<crt_device> m_crt;
	required_ioport m_spacewar;
	required_ioport m_csw;
	required_ioport m_sense;
	required_ioport m_tstadd;
	required_ioport m_twdmsb;
	required_ioport m_twdlsb;
	required_ioport m_cfg;
	required_ioport m_io_lightpen;
	required_ioport m_lightx;
	required_ioport m_lighty;
};

#endif // MAME_PDP1_PDP1_H
