// license:BSD-3-Clause
// copyright-holders:Curt Coder, Angelo Salese
/**********************************************************************

    NEC uPD3301 Programmable CRT Controller emulation

**********************************************************************
                            _____   _____
                  VRTC   1 |*    \_/     | 40  Vcc
                   RVV   2 |             | 39  SL0
                   CSR   3 |             | 38  LC0
                  LPEN   4 |             | 37  LC1
                   INT   5 |             | 36  LC2
                   DRQ   6 |             | 35  LC3
                 _DACK   7 |             | 34  VSP
                    A0   8 |             | 33  SL12
                   _RD   9 |             | 32  GPA
                   _WR  10 |   uPD3301   | 31  HLGT
                   _CS  11 |             | 30  CC7
                   DB0  12 |             | 29  CC6
                   DB1  13 |             | 28  CC5
                   DB2  14 |             | 27  CC4
                   DB3  15 |             | 26  CC3
                   DB4  16 |             | 25  CC2
                   DB5  17 |             | 24  CC1
                   DB6  18 |             | 23  CC0
                   DB7  19 |             | 22  CCLK
                   GND  20 |_____________| 21  HRTC

**********************************************************************/

#ifndef MAME_VIDEO_UPD3301_H
#define MAME_VIDEO_UPD3301_H

#pragma once



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

//#define UPD3301_DRAW_CHARACTER_MEMBER(_name) void _name(bitmap_rgb32 &bitmap, int y, int sx, uint8_t cc, uint8_t lc, int hlgt, int rvv, int vsp, int sl0, int sl12, int csr, int gpa)
#define UPD3301_DRAW_CHARACTER_MEMBER(_name) void _name(bitmap_rgb32 &bitmap, int y, int sx, uint8_t cc, uint8_t lc, int csr, bool attr_blink_on, u16 attr, u8 gfx_mode, bool is_lowestline)

#define UPD3301_FETCH_ATTRIBUTE(_name) std::array<u16, 80> _name(const std::array<u8, 41> attr_row, u8 gfx_mode, int y, u8 attr_fifo_size, u8 row_size)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> upd3301_device

class upd3301_device :  public device_t,
						public device_video_interface
{
public:
//  typedef device_delegate<void (bitmap_rgb32 &bitmap, int y, int sx, uint8_t cc, uint8_t lc, int hlgt, int rvv, int vsp, int sl0, int sl12, int csr, int gpa)> draw_character_delegate;
	typedef device_delegate<void (bitmap_rgb32 &bitmap, int y, int sx, uint8_t cc, uint8_t lc, int csr, bool attr_blink_on, u16 attr, u8 gfx_mode, bool is_lowerline)> draw_character_delegate;
	typedef device_delegate<std::array<u16, 80> (const std::array<u8, 41> attr_row, u8 gfx_mode, int y, u8 attr_fifo_size, u8 row_size)> fetch_attribute_delegate;

	// construction/destruction
	upd3301_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_character_width(int value) { m_width = value; }
	template <typename... T> void set_display_callback(T &&... args) { m_display_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_attribute_fetch_callback(T &&... args) { m_attr_fetch_cb.set(std::forward<T>(args)...); }

	UPD3301_FETCH_ATTRIBUTE( default_attr_fetch );

	auto drq_wr_callback() { return m_write_drq.bind(); }
	auto int_wr_callback() { return m_write_int.bind(); }
	auto hrtc_wr_callback() { return m_write_hrtc.bind(); }
	auto vrtc_wr_callback() { return m_write_vrtc.bind(); }
	auto rvv_wr_callback() { return m_write_rvv.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	void dack_w(uint8_t data);
	void lpen_w(int state);
	int hrtc_r();
	int vrtc_r();
	int lines_per_char() { return m_r; }
	bool is_gfx_color_mode();
	bool get_display_status();

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	TIMER_CALLBACK_MEMBER(hrtc_update);
	TIMER_CALLBACK_MEMBER(vrtc_update);

private:
	void set_interrupt(int state);
	void set_drq(int state);
	void set_display(int state);
	void reset_counters();
	void update_hrtc_timer(int state);
	void update_vrtc_timer(int state);
	void recompute_parameters();

	void draw_scanline();
	inline void reset_fifo_vrtc();

	devcb_write_line   m_write_int;
	devcb_write_line   m_write_drq;
	devcb_write_line   m_write_hrtc;
	devcb_write_line   m_write_vrtc;
	devcb_write_line   m_write_rvv;

	draw_character_delegate m_display_cb;
	fetch_attribute_delegate m_attr_fetch_cb;
	int m_width;

	// screen drawing
	bitmap_rgb32 m_bitmap;     // bitmap
	int m_y;                        // current scanline
	int m_hrtc;                     // horizontal retrace
	int m_vrtc;                     // vertical retrace

	// live state
	int m_mode;                     // command mode
	uint8_t m_status;                 // status register
	int m_param_count;              // parameter count

	// FIFOs
	u8 m_data_fifo[2][80];       // row data FIFO
	std::array<std::array<u8, 40+1>, 2> m_attr_fifo;     // attribute FIFO (+1 for extending to end of row)
	int m_data_fifo_pos;            // row data FIFO position
	int m_attr_fifo_pos;            // attribute FIFO position
	int m_input_fifo;               // which FIFO is in input mode

	// interrupts
	int m_mn;                       // disable special character interrupt
	int m_me;                       // disable end of screen interrupt
	int m_dma_mode;                 // DMA mode

	// screen geometry
	int m_h;                        // characters per line
	int m_b;                        // cursor blink time
	int m_l;                        // lines per screen
	int m_s;                        // display every other line
	int m_c;                        // cursor mode
	int m_r;                        // lines per character
	int m_v;                        // vertical blanking height
	int m_z;                        // horizontal blanking width

	// attributes
//  int m_at1;                      //
//  int m_at0;                      //
//  int m_sc;                       //
	u8  m_gfx_mode;                 // AT1 + AT0 + SC
	int m_attr;                     // attributes per row
	int m_attr_blink;               // attribute blink
	int m_attr_frame;               // attribute blink frame counter

	// cursor
	int m_cm;                       // cursor visible
	int m_cx;                       // cursor column
	int m_cy;                       // cursor row
	int m_cursor_blink;             // cursor blink
	int m_cursor_frame;             // cursor blink frame counter

	// misc
	bool m_reverse_display;

	// timers
	emu_timer *m_hrtc_timer;
	emu_timer *m_vrtc_timer;
};


// device type definition
DECLARE_DEVICE_TYPE(UPD3301, upd3301_device)

#endif // MAME_VIDEO_UPD3301_H
