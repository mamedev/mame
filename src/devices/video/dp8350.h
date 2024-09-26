// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    National Semiconductor DP8350 Series CRT Controllers

***********************************************************************
                           _____    _____
                  RSB   1 |*    \__/     | 40  Vcc
               VBLANK   2 |              | 39  RSA
             50/60 HZ   3 |              | 38  RL
                VSYNC   4 |              | 37  RAE
            FULL/HALF   5 |              | 36  A0
                  LC3   6 |              | 35  A1
                  LC2   7 |              | 34  A2
                  LC1   8 |              | 33  A3
                  LC0   9 |              | 32  A4
                  CLC  10 |              | 31  A5
                  CGP  11 |    DP835X    | 30  A6
                 LBRE  12 |              | 29  A7
                  LRC  13 |              | 28  A8
                HSYNC  14 |              | 27  A9
                RESET  15 |              | 26  A10
                  LBC  16 |              | 25  A11
                 ECLC  17 |              | 24  LCGA
                 LVSR  18 |              | 23  DRC
                  CUR  19 |              | 22  X2
                  GND  20 |______________| 21  X1

**********************************************************************/

#ifndef MAME_VIDEO_DP8350_H
#define MAME_VIDEO_DP8350_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dp835x_device

class dp835x_device : public device_t, public device_video_interface
{
public:
	// device configuration
	auto lrc_callback() { return m_lrc_callback.bind(); }
	auto clc_callback() { return m_clc_callback.bind(); }
	auto lc_callback() { return m_lc_callback.bind(); }
	auto lbre_callback() { return m_lbre_callback.bind(); }
	auto hsync_callback() { return m_hsync_callback.bind(); }
	auto vsync_callback() { return m_vsync_callback.bind(); }
	auto vblank_callback() { return m_vblank_callback.bind(); }
	void set_half_shift(bool half_shift) { m_half_shift = half_shift; }

	// write handlers
	void refresh_control(int state);
	void character_generator_program(int state);
	void register_load(u8 rs, u16 addr);

	// read handlers
	int lrc_r();
	u8 lc_r() { return m_lc; }
	int lbre_r();
	int hsync_r();
	int vsync_r();
	int vblank_r();

	// address getters (TODO: accurate character-by-character emulation)
	u16 top_of_page() const { return m_topr; }
	u16 row_start() const { return m_row_start; }
	u16 cursor_address() const { return m_cr; }

protected:
	// base type constructor
	dp835x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock,
					  int char_width, int char_height, int chars_per_row, int rows_per_frame,
					  int vsync_delay_f1, int vsync_width_f1, int vblank_interval_f1,
					  int vsync_delay_f0, int vsync_width_f0, int vblank_interval_f0,
					  int chars_per_line, int hsync_delay, int hsync_width, int vblank_stop,
					  bool cursor_on_all_lines, int lbc_0_width, int hsync_serration,
					  bool hsync_active, bool vsync_active, bool vblank_active);

	// device_t implementation
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal helpers
	void reconfigure_screen();

	// timer callbacks
	TIMER_CALLBACK_MEMBER(hblank_start);
	TIMER_CALLBACK_MEMBER(hblank_near_end);
	TIMER_CALLBACK_MEMBER(hsync_update);

	// mask parameters
	const int m_char_width;                 // character field cell size (width in dots)
	const int m_char_height;                // character field cell size (height in scan lines)
	const int m_chars_per_row;              // video characters per row
	const int m_rows_per_frame;             // video characters per frame
	const int m_vsync_delay[2];             // delay from VBLANK start to VSYNC in scan lines
	const int m_vsync_width[2];             // vertical sync width in scan lines
	const int m_vblank_interval[2];         // scan lines of vertical blanking
	const int m_chars_per_line;             // character times per scan line
	const int m_hsync_delay;                // delay from HBLANK start to HSYNC in character times
	const int m_hsync_width;                // horizontal sync width in character times
	const int m_vblank_stop;                // VBLANK stop before video start in scan lines
	const bool m_cursor_on_all_lines;       // true if cursor active on all scan lines of row
	const int m_lbc_0_width;                // width of LBC 0 output within character time
	const int m_hsync_serration;            // width of HSYNC pulse serrations within VSYNC (0 if unused)
	const bool m_hsync_active;              // active level of horizontal sync pulse
	const bool m_vsync_active;              // active level of vertical sync pulse
	const bool m_vblank_active;             // active level of vertical blanking pulse

	// derived parameters
	const int m_dots_per_line;              // number of dots per scan line
	const int m_dots_per_row;               // number of dots displayed in each scan line
	const int m_video_scan_lines;           // number of active scan lines between VBLANK periods

	// misc. configuration
	bool m_half_shift;                      // adjust screen parameters to allow half-dot shifting

	// device callbacks
	devcb_write_line m_lrc_callback;        // line rate clock output (active high)
	devcb_write_line m_clc_callback;        // clear line counter output (active low during blanking)
	devcb_write8 m_lc_callback;             // line counter output
	devcb_write_line m_lbre_callback;       // line buffer recirculate enable output (active high)
	devcb_write_line m_hsync_callback;      // horizontal sync output (polarity may vary by type)
	devcb_write_line m_vsync_callback;      // vertical sync output (polarity may vary by type)
	devcb_write_line m_vblank_callback;     // vertical blanking output (polarity may vary by type)

	// internal registers and control parameters
	bool m_60hz_refresh;                    // refresh rate selector (true = f1, false = f0)
	bool m_cgpi;                            // character generator program/address mode input
	u16 m_topr;                             // top-of-page register
	u16 m_rsr;                              // row start register (to be loaded during next row)
	u16 m_cr;                               // cursor register
	u16 m_row_start;                        // start of current row
	u16 m_line;                             // current scan line (starting at HBLANK)

	// output state
	u8 m_lc;                                // 4-bit line counter

	// timers
	emu_timer *m_hblank_start_timer;
	emu_timer *m_hblank_near_end_timer;
	emu_timer *m_hsync_on_timer;
	emu_timer *m_hsync_off_timer;
};

// ======================> dp8350_device

class dp8350_device : public dp835x_device
{
public:
	// device constructor
	dp8350_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// ======================> dp8367_device

class dp8367_device : public dp835x_device
{
public:
	// device constructor
	dp8367_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// ======================> dp835x_a_device

class dp835x_a_device : public dp835x_device
{
public:
	// device constructor
	dp835x_a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// device type declarations
DECLARE_DEVICE_TYPE(DP8350, dp8350_device)
DECLARE_DEVICE_TYPE(DP8367, dp8367_device)
DECLARE_DEVICE_TYPE(DP835X_A, dp835x_a_device)

#endif // MAME_VIDEO_DP8350_H
