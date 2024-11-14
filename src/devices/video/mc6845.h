// license:BSD-3-Clause
// copyright-holders:Wilbert Pol,Nigel Barnes
/**********************************************************************

    Motorola MC6845 and compatible CRT controller emulation

***********************************************************************
                            ____    ____
                   GND   1 |*   \__/    | 40  VS
                _RESET   2 |            | 39  HS
                 LPSTB   3 |            | 38  RA0
                   MA0   4 |            | 37  RA1
                   MA1   5 |            | 36  RA2
                   MA2   6 |            | 35  RA3
                   MA3   7 |            | 34  RA4
                   MA4   8 |            | 33  D0
                   MA5   9 |            | 32  D1
                   MA6  10 |            | 31  D2
                   MA7  11 |   MC6845   | 30  D3
                   MA8  12 |            | 29  D4
                   MA9  13 |            | 28  D5
                  MA10  14 |            | 27  D6
                  MA11  15 |            | 26  D7
                  MA12  16 |            | 25  _CS
                  MA13  17 |            | 24  RS
                    DE  18 |            | 23  E
                CURSOR  19 |            | 22  R/_W
                   Vcc  20 |____________| 21  CLK

**********************************************************************/

#ifndef MAME_VIDEO_MC6845_H
#define MAME_VIDEO_MC6845_H

#pragma once


/* callback definitions */
#define MC6845_RECONFIGURE(name)  void name(int width, int height, const rectangle &visarea, attoseconds_t frame_period)

#define MC6845_BEGIN_UPDATE(name)  void name(bitmap_rgb32 &bitmap, const rectangle &cliprect)

#define MC6845_UPDATE_ROW(name)     void name(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint16_t ma, uint8_t ra, \
												uint16_t y, uint8_t x_count, int8_t cursor_x, int de, int hbp, int vbp)

#define MC6845_END_UPDATE(name)     void name(bitmap_rgb32 &bitmap, const rectangle &cliprect)

#define MC6845_ON_UPDATE_ADDR_CHANGED(name) void name(int address, int strobe)


class mc6845_device :   public device_t,
						public device_video_interface
{
public:
	typedef device_delegate<void (int width, int height, const rectangle &visarea, attoseconds_t frame_period)> reconfigure_delegate;
	typedef device_delegate<void (bitmap_rgb32 &bitmap, const rectangle &cliprect)> begin_update_delegate;
	typedef device_delegate<void (bitmap_rgb32 &bitmap, const rectangle &cliprect, uint16_t ma, uint8_t ra,
									uint16_t y, uint8_t x_count, int8_t cursor_x, int de, int hbp, int vbp)> update_row_delegate;
	typedef device_delegate<void (bitmap_rgb32 &bitmap, const rectangle &cliprect)> end_update_delegate;
	typedef device_delegate<void (int address, int strobe)> on_update_addr_changed_delegate;

	// construction/destruction
	mc6845_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// HACK: show the full screen_device htotal/vtotal if true (regulate with UI slider controls)
	void set_show_border_area(bool show) { m_show_border_area = show; }
	// HACK: a static alternative of above, potentially unsafe.
	void set_visarea_adjust(int min_x, int max_x, int min_y, int max_y)
	{
		m_visarea_adjust_min_x = min_x;
		m_visarea_adjust_max_x = max_x;
		m_visarea_adjust_min_y = min_y;
		m_visarea_adjust_max_y = max_y;
	}
	void set_char_width(int pixels) { m_hpixels_per_column = pixels; }

	template <typename... T> void set_reconfigure_callback(T &&... args) { m_reconfigure_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_begin_update_callback(T &&... args) { m_begin_update_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_update_row_callback(T &&... args) { m_update_row_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_end_update_callback(T &&... args) { m_end_update_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_on_update_addr_change_callback(T &&... args) { m_on_update_addr_changed_cb.set(std::forward<T>(args)...); }

	auto out_de_callback() { return m_out_de_cb.bind(); }
	auto out_cur_callback() { return m_out_cur_cb.bind(); }
	auto out_hsync_callback() { return m_out_hsync_cb.bind(); }
	auto out_vsync_callback() { return m_out_vsync_cb.bind(); }

	/* select one of the registers for reading or writing */
	void address_w(uint8_t data);

	/* read from the status register */
	uint8_t status_r();

	/* read from the currently selected register */
	uint8_t register_r();

	/* write to the currently selected register */
	void register_w(uint8_t data);

	// read display enable line state
	int de_r();

	// read cursor line state
	int cursor_r();

	// read horizontal sync line state
	int hsync_r();

	// read vertical sync line state
	int vsync_r();

	/* return the current value on the MA0-MA13 pins */
	uint16_t get_ma();

	/* return the current value on the RA0-RA4 pins */
	uint8_t get_ra();

	/* simulates the LO->HI clocking of the light pen pin (pin 3) */
	void assert_light_pen_input();
	void assert_light_pen_input(u16);

	/* set number of pixels per video memory address */
	void set_hpixels_per_column(int hpixels_per_column);

	/* updates the screen -- this will call begin_update(),
	   followed by update_row() repeatedly and after all row
	   updating is complete, end_update() */
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	mc6845_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;
	virtual void device_clock_changed() override;

	attotime cclks_to_attotime(uint64_t clocks) const { return clocks_to_attotime(clocks * m_clk_scale); }
	uint64_t attotime_to_cclks(const attotime &duration) const { return attotime_to_clocks(duration) / m_clk_scale; }

	bool m_supports_disp_start_addr_r;
	bool m_supports_vert_sync_width;
	bool m_supports_status_reg_d5;
	bool m_supports_status_reg_d6;
	bool m_supports_status_reg_d7;
	bool m_supports_transparent;

	/* register file */
	uint8_t   m_horiz_char_total;     /* 0x00 */
	uint8_t   m_horiz_disp;           /* 0x01 */
	uint8_t   m_horiz_sync_pos;       /* 0x02 */
	uint8_t   m_sync_width;           /* 0x03 */
	uint8_t   m_vert_char_total;      /* 0x04 */
	uint8_t   m_vert_total_adj;       /* 0x05 */
	uint8_t   m_vert_disp;            /* 0x06 */
	uint8_t   m_vert_sync_pos;        /* 0x07 */
	uint8_t   m_mode_control;         /* 0x08 */
	uint8_t   m_max_ras_addr;         /* 0x09 */
	uint8_t   m_cursor_start_ras;     /* 0x0a */
	uint8_t   m_cursor_end_ras;       /* 0x0b */
	uint16_t  m_disp_start_addr;      /* 0x0c/0x0d */
	uint16_t  m_cursor_addr;          /* 0x0e/0x0f */
	uint16_t  m_light_pen_addr;       /* 0x10/0x11 */
	uint16_t  m_update_addr;          /* 0x12/0x13 */

	/* other internal state */
	uint8_t   m_register_address_latch;
	bool    m_cursor_state;
	uint8_t   m_cursor_blink_count;
	bool    m_update_ready_bit;
	/* output signals */
	int     m_cur;
	int     m_hsync;
	int     m_vsync;
	int     m_de;

	/* internal counters */
	uint8_t   m_character_counter;        /* Not used yet */
	uint8_t   m_hsync_width_counter;  /* Not used yet */
	uint8_t   m_line_counter;
	uint8_t   m_raster_counter;
	uint8_t   m_adjust_counter;
	uint8_t   m_vsync_width_counter;

	bool    m_line_enable_ff;       /* Internal flip flop which is set when the line_counter is reset and reset when vert_disp is reached */
	uint8_t   m_vsync_ff;
	uint8_t   m_adjust_active;
	uint16_t  m_line_address;
	int16_t   m_cursor_x;

	/* timers */
	emu_timer *m_line_timer;
	emu_timer *m_de_off_timer;
	emu_timer *m_cursor_on_timer;
	emu_timer *m_cursor_off_timer;
	emu_timer *m_hsync_on_timer;
	emu_timer *m_hsync_off_timer;
	emu_timer *m_light_pen_latch_timer;
	emu_timer *m_upd_adr_timer;
	emu_timer *m_upd_trans_timer;

	/* computed values - do NOT state save these! */
	/* These computed are used to define the screen parameters for a driver */
	uint16_t  m_horiz_pix_total;
	uint16_t  m_vert_pix_total;
	uint16_t  m_max_visible_x;
	uint16_t  m_max_visible_y;
	uint16_t  m_hsync_on_pos;
	uint16_t  m_hsync_off_pos;
	uint16_t  m_vsync_on_pos;
	uint16_t  m_vsync_off_pos;
	bool    m_has_valid_parameters;
	bool    m_display_disabled_msg_shown;

	uint16_t   m_current_disp_addr;   /* the display address currently drawn (used only in mc6845_update) */

	bool     m_light_pen_latched;
	attotime m_upd_time;

	void update_upd_adr_timer();
	void call_on_update_address(int strobe);
	void transparent_update();
	void recompute_parameters(bool postload);
	void update_counters();
	void set_de(int state);
	void set_hsync(int state);
	void set_vsync(int state);
	void set_cur(int state);
	bool match_line();
	virtual bool check_cursor_visible(uint16_t ra, uint16_t line_addr);
	TIMER_CALLBACK_MEMBER(handle_line_timer);
	TIMER_CALLBACK_MEMBER(de_off_tick);
	TIMER_CALLBACK_MEMBER(cursor_on);
	TIMER_CALLBACK_MEMBER(cursor_off);
	TIMER_CALLBACK_MEMBER(hsync_on);
	TIMER_CALLBACK_MEMBER(hsync_off);
	TIMER_CALLBACK_MEMBER(latch_light_pen);
	TIMER_CALLBACK_MEMBER(adr_update_tick);
	TIMER_CALLBACK_MEMBER(transparent_update_tick);
	virtual void update_cursor_state();
	virtual uint8_t draw_scanline(int y, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	/************************
	 interface CRTC - driver
	 ************************/

	bool m_show_border_area;        /* visible screen area (false) active display (true) active display + blanking */
	int m_noninterlace_adjust;      /* adjust max ras in non-interlace mode */
	int m_interlace_adjust;         /* adjust max ras in interlace mode */

	uint32_t m_clk_scale;

	/* visible screen area adjustment */
	int m_visarea_adjust_min_x;
	int m_visarea_adjust_max_x;
	int m_visarea_adjust_min_y;
	int m_visarea_adjust_max_y;

	int m_hpixels_per_column;       /* number of pixels per video memory address */

	reconfigure_delegate m_reconfigure_cb;

	/* if specified, this gets called before any pixel update,
	 optionally return a pointer that will be passed to the
	 update and tear down callbacks */
	begin_update_delegate m_begin_update_cb;

	/* this gets called for every row, the driver must output
	 x_count * hpixels_per_column pixels.
	 cursor_x indicates the character position where the cursor is, or -1
	 if there is no cursor on this row */
	update_row_delegate  m_update_row_cb;

	/* if specified, this gets called after all row updating is complete */
	end_update_delegate  m_end_update_cb;

	/* Called whenever the update address changes
	 * For vblank/hblank timing strobe indicates the physical update.
	 * vblank/hblank timing not supported yet! */
	on_update_addr_changed_delegate m_on_update_addr_changed_cb;

	/* if specified, this gets called for every change of the display enable pin (pin 18) */
	devcb_write_line            m_out_de_cb;

	/* if specified, this gets called for every change of the cursor pin (pin 19) */
	devcb_write_line            m_out_cur_cb;

	/* if specified, this gets called for every change of the HSYNC pin (pin 39) */
	devcb_write_line            m_out_hsync_cb;

	/* if specified, this gets called for every change of the VSYNC pin (pin 40) */
	devcb_write_line            m_out_vsync_cb;
};


class mc6845_1_device : public mc6845_device
{
public:
	mc6845_1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};

class r6545_1_device : public mc6845_device
{
public:
	r6545_1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};

class c6545_1_device : public mc6845_device
{
public:
	c6545_1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};

class hd6845s_device : public mc6845_device
{
public:
	hd6845s_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	hd6845s_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual bool check_cursor_visible(uint16_t ra, uint16_t line_addr) override;
};

class sy6545_1_device : public mc6845_device
{
public:
	sy6545_1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};

class sy6845e_device : public mc6845_device
{
public:
	sy6845e_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};

class hd6345_device : public hd6845s_device
{
public:
	hd6345_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void address_w(uint8_t data);
	uint8_t register_r();
	void register_w(uint8_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	/* register file */
	uint8_t   m_disp2_pos;            /* 0x12 */
	uint16_t  m_disp2_start_addr;     /* 0x13/0x14 */
	uint8_t   m_disp3_pos;            /* 0x15 */
	uint16_t  m_disp3_start_addr;     /* 0x16/0x17 */
	uint8_t   m_disp4_pos;            /* 0x18 */
	uint16_t  m_disp4_start_addr;     /* 0x19/0x1a */
	uint8_t   m_vert_sync_pos_adj;    /* 0x1b */
	uint8_t   m_smooth_scroll_ras;    /* 0x1d */
	uint8_t   m_control1;             /* 0x1e */
	uint8_t   m_control2;             /* 0x1f */
	uint8_t   m_control3;             /* 0x20 */
	uint8_t   m_mem_width_offs;       /* 0x21 */
	uint8_t   m_cursor2_start_ras;    /* 0x22 */
	uint8_t   m_cursor2_end_ras;      /* 0x23 */
	uint16_t  m_cursor2_addr;         /* 0x24/0x25 */
	uint8_t   m_cursor_width;         /* 0x26 */
	uint8_t   m_cursor2_width;        /* 0x27 */

	virtual uint8_t draw_scanline(int y, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;
};

class ams40489_device : public mc6845_device
{
public:
	ams40489_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};


DECLARE_DEVICE_TYPE(MC6845,   mc6845_device)
DECLARE_DEVICE_TYPE(MC6845_1, mc6845_1_device)
DECLARE_DEVICE_TYPE(R6545_1,  r6545_1_device)
DECLARE_DEVICE_TYPE(C6545_1,  c6545_1_device)
DECLARE_DEVICE_TYPE(HD6845S,  hd6845s_device)
DECLARE_DEVICE_TYPE(SY6545_1, sy6545_1_device)
DECLARE_DEVICE_TYPE(SY6845E,  sy6845e_device)
DECLARE_DEVICE_TYPE(HD6345,   hd6345_device)
DECLARE_DEVICE_TYPE(AMS40489, ams40489_device)

#endif // MAME_VIDEO_MC6845_H
