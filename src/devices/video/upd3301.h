// license:BSD-3-Clause
// copyright-holders:Curt Coder
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

#pragma once

#ifndef __UPD3301__
#define __UPD3301__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define UPD3301_DRAW_CHARACTER_MEMBER(_name) void _name(bitmap_rgb32 &bitmap, int y, int sx, UINT8 cc, UINT8 lc, int hlgt, int rvv, int vsp, int sl0, int sl12, int csr, int gpa)


#define MCFG_UPD3301_CHARACTER_WIDTH(_value) \
	upd3301_device::static_set_character_width(*device, _value);

#define MCFG_UPD3301_DRAW_CHARACTER_CALLBACK_OWNER(_class, _method) \
	upd3301_device::static_set_display_callback(*device, upd3301_draw_character_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_UPD3301_DRQ_CALLBACK(_write) \
	devcb = &upd3301_device::set_drq_wr_callback(*device, DEVCB_##_write);

#define MCFG_UPD3301_INT_CALLBACK(_write) \
	devcb = &upd3301_device::set_int_wr_callback(*device, DEVCB_##_write);

#define MCFG_UPD3301_HRTC_CALLBACK(_write) \
	devcb = &upd3301_device::set_hrtc_wr_callback(*device, DEVCB_##_write);

#define MCFG_UPD3301_VRTC_CALLBACK(_write) \
	devcb = &upd3301_device::set_vrtc_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

typedef device_delegate<void (bitmap_rgb32 &bitmap, int y, int sx, UINT8 cc, UINT8 lc, int hlgt, int rvv, int vsp, int sl0, int sl12, int csr, int gpa)> upd3301_draw_character_delegate;


// ======================> upd3301_device

class upd3301_device :  public device_t,
						public device_video_interface
{
public:
	// construction/destruction
	upd3301_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	static void static_set_character_width(device_t &device, int value) { downcast<upd3301_device &>(device).m_width = value; }
	static void static_set_display_callback(device_t &device, upd3301_draw_character_delegate callback) { downcast<upd3301_device &>(device).m_display_cb = callback; }

	template<class _Object> static devcb_base &set_drq_wr_callback(device_t &device, _Object object) { return downcast<upd3301_device &>(device).m_write_drq.set_callback(object); }
	template<class _Object> static devcb_base &set_int_wr_callback(device_t &device, _Object object) { return downcast<upd3301_device &>(device).m_write_int.set_callback(object); }
	template<class _Object> static devcb_base &set_hrtc_wr_callback(device_t &device, _Object object) { return downcast<upd3301_device &>(device).m_write_hrtc.set_callback(object); }
	template<class _Object> static devcb_base &set_vrtc_wr_callback(device_t &device, _Object object) { return downcast<upd3301_device &>(device).m_write_vrtc.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_WRITE8_MEMBER( dack_w );
	DECLARE_WRITE_LINE_MEMBER( lpen_w );
	DECLARE_READ_LINE_MEMBER( hrtc_r );
	DECLARE_READ_LINE_MEMBER( vrtc_r );

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_clock_changed() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	enum
	{
		TIMER_HRTC,
		TIMER_VRTC,
		TIMER_DRQ
	};

	void set_interrupt(int state);
	void set_drq(int state);
	void set_display(int state);
	void reset_counters();
	void update_hrtc_timer(int state);
	void update_vrtc_timer(int state);
	void recompute_parameters();

	void draw_scanline();

	devcb_write_line   m_write_int;
	devcb_write_line   m_write_drq;
	devcb_write_line   m_write_hrtc;
	devcb_write_line   m_write_vrtc;

	upd3301_draw_character_delegate m_display_cb;
	int m_width;

	// screen drawing
	bitmap_rgb32 *m_bitmap;     // bitmap
	int m_y;                        // current scanline
	int m_hrtc;                     // horizontal retrace
	int m_vrtc;                     // vertical retrace

	// live state
	int m_mode;                     // command mode
	UINT8 m_status;                 // status register
	int m_param_count;              // parameter count

	// FIFOs
	UINT8 m_data_fifo[80][2];       // row data FIFO
	UINT8 m_attr_fifo[40][2];       // attribute FIFO
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
	int m_at1;                      //
	int m_at0;                      //
	int m_sc;                       //
	int m_attr;                     // attributes per row
	int m_attr_blink;               // attribute blink
	int m_attr_frame;               // attribute blink frame counter

	// cursor
	int m_cm;                       // cursor visible
	int m_cx;                       // cursor column
	int m_cy;                       // cursor row
	int m_cursor_blink;             // cursor blink
	int m_cursor_frame;             // cursor blink frame counter

	// timers
	emu_timer *m_hrtc_timer;
	emu_timer *m_vrtc_timer;
	emu_timer *m_drq_timer;
};


// device type definition
extern const device_type UPD3301;



#endif
