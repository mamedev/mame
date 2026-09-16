// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    IBM EGA CRT Controller emulation

**********************************************************************/

#ifndef MAME_VIDEO_CRTC_EGA_H
#define MAME_VIDEO_CRTC_EGA_H


#define CRTC_EGA_BEGIN_UPDATE(_name) void _name(bitmap_ind16 &bitmap, const rectangle &cliprect)
#define CRTC_EGA_PIXEL_UPDATE(_name)   void _name(bitmap_ind16 &bitmap,    \
												const rectangle &cliprect, uint16_t ma, uint8_t ra,                 \
												uint16_t y, uint8_t x, int8_t cursor_x)
#define CRTC_EGA_END_UPDATE(_name)   void _name(bitmap_ind16 &bitmap, const rectangle &cliprect)


class crtc_ega_device : public device_t,
						public device_video_interface
{
public:
	/* callback definitions */
	typedef device_delegate<void (bitmap_ind16 &bitmap, const rectangle &cliprect)> begin_update_delegate;
	typedef device_delegate<void (bitmap_ind16 &bitmap, const rectangle &cliprect, uint16_t ma, uint8_t ra, uint16_t y, uint8_t x_count, int8_t cursor_x)> row_update_delegate;
	typedef device_delegate<void (bitmap_ind16 &bitmap, const rectangle &cliprect)> end_update_delegate;


	crtc_ega_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto res_out_de_callback() { return m_res_out_de_cb.bind(); }
	auto res_out_hsync_callback() { return m_res_out_hsync_cb.bind(); }
	auto res_out_vsync_callback() { return m_res_out_vsync_cb.bind(); }
	auto res_out_vblank_callback() { return m_res_out_vblank_cb.bind(); }
	auto res_out_irq_callback() { return m_res_out_irq_cb.bind(); }

	template <typename... T> void set_begin_update_callback(T &&... args) { m_begin_update_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_row_update_callback(T &&... args) { m_row_update_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_end_update_callback(T &&... args) { m_end_update_cb.set(std::forward<T>(args)...); }
	void config_set_hpixels_per_column(int hpixels_per_column) { m_hpixels_per_column = hpixels_per_column; }

	/* select one of the registers for reading or writing */
	void address_w(uint8_t data);

	/* read from the currently selected register */
	uint8_t register_r();

	/* write to the currently selected register */
	void register_w(uint8_t data);

	/* return the current value on the MA0-MA15 pins */
	uint16_t get_ma();

	/* return the current value on the RA0-RA4 pins */
	uint8_t get_ra();

	/* simulates the LO->HI clocking of the light pen pin */
	void assert_light_pen_input();

	/* set the clock of the chip */
	void set_clock(int clock);

	/* set number of pixels per video memory address */
	void set_hpixels_per_column(int hpixels_per_column);

	/* updates the screen -- this will call begin_update(), */
	/* followed by update_row() repeatedly and after all row */
	/* updating is complete, end_update() */
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

private:
	devcb_write_line   m_res_out_de_cb;
	devcb_write_line   m_res_out_hsync_cb;
	devcb_write_line   m_res_out_vsync_cb;
	devcb_write_line   m_res_out_vblank_cb;
	devcb_write_line   m_res_out_irq_cb;

	/* if specified, this gets called before any pixel update,
	 optionally return a pointer that will be passed to the
	 update and tear down callbacks */
	begin_update_delegate      m_begin_update_cb;

	/* this gets called for every row, the driver must output
	 x_count * hpixels_per_column pixels.
	 cursor_x indicates the character position where the cursor is, or -1
	 if there is no cursor on this row */
	row_update_delegate        m_row_update_cb;

	/* if specified, this gets called after all row updating is complete */
	end_update_delegate        m_end_update_cb;

	/* ega/vga register file */
	uint8_t   m_horiz_char_total; /* 0x00 */
	uint8_t   m_horiz_disp;           /* 0x01 */
	uint8_t   m_horiz_blank_start;    /* 0x02 */
	uint8_t   m_horiz_blank_end;  /* 0x03/0x05 */
	uint8_t   m_ena_vert_access;  /* 0x03 */
	uint8_t   m_de_skew;          /* 0x03 */
	uint8_t   m_horiz_retr_start; /* 0x04 */
	uint8_t   m_horiz_retr_end;       /* 0x05 */
	uint8_t   m_horiz_retr_skew;  /* 0x05 */
	uint16_t  m_vert_total;           /* 0x06/0x07 */
	uint8_t   m_preset_row_scan;  /* 0x08 */
	uint8_t   m_max_ras_addr;     /* 0x09 */
	uint8_t   m_cursor_start_ras; /* 0x0a */
	uint8_t   m_cursor_disable;       /* 0x0a */
	uint8_t   m_cursor_end_ras;       /* 0x0b */
	uint8_t   m_cursor_skew;      /* 0x0b */
	uint16_t  m_disp_start_addr;  /* 0x0c/0x0d */
	uint16_t  m_cursor_addr;      /* 0x0e/0x0f */
	uint16_t  m_light_pen_addr;       /* 0x10/0x11 */
	uint16_t  m_vert_retr_start;  /* 0x10/0x07 */
	uint8_t   m_vert_retr_end;        /* 0x11 */
	uint8_t   m_irq_enable;            /* 0x11 */
	uint16_t  m_vert_disp_end;        /* 0x12/0x07 */
	uint8_t   m_offset;               /* 0x13 */
	uint8_t   m_underline_loc;        /* 0x14 */
	uint16_t  m_vert_blank_start; /* 0x15/0x07/0x09 */
	uint8_t   m_vert_blank_end;       /* 0x16 */
	uint8_t   m_mode_control;     /* 0x17 */
	uint16_t  m_line_compare;     /* 0x18/0x07/0x09 */

	/* other internal state */
	uint8_t   m_register_address_latch;
	uint16_t  m_start_addr_latch;
	uint8_t   m_preset_row_latch;
	bool    m_cursor_state; /* 0 = off, 1 = on */
	uint8_t   m_cursor_blink_count;
	int     m_hpixels_per_column;       /* number of pixels per video memory address */

	/* output signals */
	int     m_cur;
	int     m_hsync;
	int     m_vsync;
	int     m_vblank;
	int     m_de;

	/* internal counters */
	uint8_t   m_character_counter;
	uint8_t   m_hsync_width_counter;
	uint16_t  m_line_counter;
	uint8_t   m_raster_counter;
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

	/* computed values - do NOT state save these! */
	uint16_t  m_horiz_pix_total;
	uint16_t  m_vert_pix_total;
	uint16_t  m_max_visible_x;
	uint16_t  m_max_visible_y;
	uint16_t  m_hsync_on_pos;
	uint16_t  m_hsync_off_pos;
	uint16_t  m_vsync_on_pos;
	uint16_t  m_vsync_off_pos;
	uint8_t   m_light_pen_latched;
	bool    m_has_valid_parameters;

	void recompute_parameters(bool postload);
	void update_counters();
	void set_de(int state);
	void set_hsync(int state);
	void set_vsync(int state);
	void set_vblank(int state);
	void set_cur(int state);
	TIMER_CALLBACK_MEMBER(handle_line_timer);
	TIMER_CALLBACK_MEMBER(de_off_tick);
	TIMER_CALLBACK_MEMBER(cursor_on);
	TIMER_CALLBACK_MEMBER(cursor_off);
	TIMER_CALLBACK_MEMBER(hsync_on);
	TIMER_CALLBACK_MEMBER(hsync_off);
	TIMER_CALLBACK_MEMBER(latch_light_pen);
	void update_cursor_state();
};

DECLARE_DEVICE_TYPE(CRTC_EGA, crtc_ega_device)

#endif // MAME_VIDEO_CRTC_EGA_H
