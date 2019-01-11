// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA CDP1864C COS/MOS PAL Compatible Color TV Interface

**********************************************************************
                            _____   _____
                INLACE   1 |*    \_/     | 40  Vdd
               CLK IN_   2 |             | 39  AUD
              CLR OUT_   3 |             | 38  CLR IN_
                   AOE   4 |             | 37  DMA0_
                   SC1   5 |             | 36  INT_
                   SC0   6 |             | 35  TPA
                  MRD_   7 |             | 34  TPB
                 BUS 7   8 |             | 33  EVS
                 BUS 6   9 |             | 32  V SYNC
                 BUS 5  10 |   CDP1864   | 31  H SYNC
                 BUS 4  11 |             | 30  C SYNC_
                 BUS 3  12 |             | 29  RED
                 BUS 2  13 |             | 28  BLUE
                 BUS 1  14 |             | 27  GREEN
                 BUS 0  15 |             | 26  BCK GND_
                  CON_  16 |             | 25  BURST
                    N2  17 |             | 24  ALT
                   EF_  18 |             | 23  R DATA
                    N0  19 |             | 22  G DATA
                   Vss  20 |_____________| 21  B DATA


           http://homepage.mac.com/ruske/cosmacelf/cdp1864.pdf

**********************************************************************/

#ifndef MAME_SOUND_CDP1864_H
#define MAME_SOUND_CDP1864_H

#pragma once

#include "machine/rescap.h"
#include "video/resnet.h"

#include "screen.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define CDP1864_CLOCK   XTAL(1'750'000)



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CDP1864_ADD(_tag, _screen_tag, _clock, _inlace, _irq, _dma_out, _efx, _hsync, _rdata, _bdata, _gdata) \
		MCFG_DEVICE_ADD(_tag, CDP1864, _clock) \
		MCFG_VIDEO_SET_SCREEN(_screen_tag) \
		downcast<cdp1864_device *>(device)->set_inlace_callback(DEVCB_##_inlace); \
		downcast<cdp1864_device *>(device)->set_irq_callback(DEVCB_##_irq); \
		downcast<cdp1864_device *>(device)->set_dma_out_callback(DEVCB_##_dma_out); \
		downcast<cdp1864_device *>(device)->set_efx_callback(DEVCB_##_efx); \
		downcast<cdp1864_device *>(device)->set_hsync_callback(DEVCB_##_hsync); \
		downcast<cdp1864_device *>(device)->set_rdata_callback(DEVCB_##_rdata); \
		downcast<cdp1864_device *>(device)->set_bdata_callback(DEVCB_##_bdata); \
		downcast<cdp1864_device *>(device)->set_gdata_callback(DEVCB_##_gdata);

#define MCFG_CDP1864_CHROMINANCE(_r, _b, _g, _bkg) \
		downcast<cdp1864_device *>(device)->set_chrominance_resistors(_r, _b, _g, _bkg);

#define MCFG_CDP1864_SCREEN_ADD(_tag, _clock) \
		MCFG_SCREEN_ADD(_tag, RASTER) \
		MCFG_SCREEN_RAW_PARAMS(_clock, cdp1864_device::SCREEN_WIDTH, cdp1864_device::HBLANK_END, cdp1864_device::HBLANK_START, cdp1864_device::TOTAL_SCANLINES, cdp1864_device::SCANLINE_VBLANK_END, cdp1864_device::SCANLINE_VBLANK_START)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cdp1864_device

class cdp1864_device :  public device_t,
						public device_sound_interface,
						public device_video_interface
{
public:
	static constexpr unsigned VISIBLE_COLUMNS = 64;
	static constexpr unsigned VISIBLE_LINES   = 192;

	static constexpr unsigned HBLANK_END      =  1 * 8;
	static constexpr unsigned HBLANK_START    = 13 * 8;
	static constexpr unsigned HSYNC_START     =  0 * 8;
	static constexpr unsigned HSYNC_END       =  1 * 8;
	static constexpr unsigned SCREEN_START    =  4 * 8;
	static constexpr unsigned SCREEN_END      = 12 * 8;
	static constexpr unsigned SCREEN_WIDTH    = 14 * 8;

	static constexpr unsigned TOTAL_SCANLINES             = 312;

	static constexpr unsigned SCANLINE_VBLANK_START       = TOTAL_SCANLINES - 4;
	static constexpr unsigned SCANLINE_VBLANK_END         = 20;
	static constexpr unsigned SCANLINE_VSYNC_START        = 0;
	static constexpr unsigned SCANLINE_VSYNC_END          = 4;
	static constexpr unsigned SCANLINE_DISPLAY_START      = 60; // ???
	static constexpr unsigned SCANLINE_DISPLAY_END        = SCANLINE_DISPLAY_START + VISIBLE_LINES;
	static constexpr unsigned SCANLINE_INT_START          = SCANLINE_DISPLAY_START - 2;
	static constexpr unsigned SCANLINE_INT_END            = SCANLINE_DISPLAY_START;
	static constexpr unsigned SCANLINE_EFX_TOP_START      = SCANLINE_DISPLAY_START - 4;
	static constexpr unsigned SCANLINE_EFX_TOP_END        = SCANLINE_DISPLAY_START;
	static constexpr unsigned SCANLINE_EFX_BOTTOM_START   = SCANLINE_DISPLAY_END - 4;
	static constexpr unsigned SCANLINE_EFX_BOTTOM_END     = SCANLINE_DISPLAY_END;

	// construction/destruction
	cdp1864_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> void set_inlace_callback(Object &&inlace) { m_read_inlace.set_callback(std::forward<Object>(inlace)); }
	template <class Object> void set_irq_callback(Object &&irq) { m_write_irq.set_callback(std::forward<Object>(irq)); }
	template <class Object> void set_dma_out_callback(Object &&dma_out) { m_write_dma_out.set_callback(std::forward<Object>(dma_out)); }
	template <class Object> void set_efx_callback(Object &&efx) { m_write_efx.set_callback(std::forward<Object>(efx)); }
	template <class Object> void set_hsync_callback(Object &&hsync) { m_write_hsync.set_callback(std::forward<Object>(hsync)); }
	template <class Object> void set_rdata_callback(Object &&rdata) { m_read_rdata.set_callback(std::forward<Object>(rdata)); }
	template <class Object> void set_bdata_callback(Object &&bdata) { m_read_bdata.set_callback(std::forward<Object>(bdata)); }
	template <class Object> void set_gdata_callback(Object &&gdata) { m_read_gdata.set_callback(std::forward<Object>(gdata)); }
	void set_chrominance_resistors(double r, double b, double g, double bkg) { m_chr_r = r; m_chr_b = b; m_chr_g = g; m_chr_bkg = bkg; }

	DECLARE_READ8_MEMBER( dispon_r );
	DECLARE_READ8_MEMBER( dispoff_r );

	DECLARE_WRITE8_MEMBER( step_bgcolor_w );
	DECLARE_WRITE8_MEMBER( tone_latch_w );

	DECLARE_WRITE8_MEMBER( dma_w );

	DECLARE_WRITE_LINE_MEMBER( con_w );
	DECLARE_WRITE_LINE_MEMBER( aoe_w );
	DECLARE_WRITE_LINE_MEMBER( evs_w );

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// internal callbacks
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	enum
	{
		TIMER_INT,
		TIMER_EFX,
		TIMER_DMA,
		TIMER_HSYNC
	};

	void initialize_palette();

	static constexpr int bckgnd[4] = { 2, 0, 4, 1 };

	devcb_read_line        m_read_inlace;
	devcb_read_line        m_read_rdata;
	devcb_read_line        m_read_bdata;
	devcb_read_line        m_read_gdata;
	devcb_write_line       m_write_irq;
	devcb_write_line       m_write_dma_out;
	devcb_write_line       m_write_efx;
	devcb_write_line       m_write_hsync;

	bitmap_rgb32 m_bitmap;          // bitmap
	sound_stream *m_stream;         // sound output

	// video state
	double m_chr_r;             // red chrominance resistor value
	double m_chr_b;             // blue chrominance resistor value
	double m_chr_g;             // green chrominance resistor value
	double m_chr_bkg;           // background chrominance resistor value

	rgb_t m_palette[16];
	int m_disp;                     // display on
	int m_dmaout;                   // DMA request active
	int m_bgcolor;                  // background color
	int m_con;                      // color on

	// sound state
	int m_aoe;                      // audio on
	int m_latch;                    // sound latch
	int16_t m_signal;                 // current signal
	int m_incr;                     // initial wave state

	// timers
	emu_timer *m_int_timer;
	emu_timer *m_efx_timer;
	emu_timer *m_dma_timer;
	emu_timer *m_hsync_timer;
};


// device type definition
DECLARE_DEVICE_TYPE(CDP1864, cdp1864_device)

#endif // MAME_DEVICES_SOUND_CDP1864_H
