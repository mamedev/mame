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

#pragma once

#ifndef __CDP1864__
#define __CDP1864__

#include "emu.h"
#include "machine/rescap.h"
#include "video/resnet.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define CDP1864_CLOCK   XTAL_1_75MHz

#define CDP1864_VISIBLE_COLUMNS 64
#define CDP1864_VISIBLE_LINES   192

#define CDP1864_HBLANK_END       1 * 8
#define CDP1864_HBLANK_START    13 * 8
#define CDP1864_HSYNC_START      0 * 8
#define CDP1864_HSYNC_END        1 * 8
#define CDP1864_SCREEN_START     4 * 8
#define CDP1864_SCREEN_END      12 * 8
#define CDP1864_SCREEN_WIDTH    14 * 8

#define CDP1864_TOTAL_SCANLINES             312

#define CDP1864_SCANLINE_VBLANK_START       CDP1864_TOTAL_SCANLINES - 4
#define CDP1864_SCANLINE_VBLANK_END         20
#define CDP1864_SCANLINE_VSYNC_START        0
#define CDP1864_SCANLINE_VSYNC_END          4
#define CDP1864_SCANLINE_DISPLAY_START      60 // ???
#define CDP1864_SCANLINE_DISPLAY_END        CDP1864_SCANLINE_DISPLAY_START + CDP1864_VISIBLE_LINES
#define CDP1864_SCANLINE_INT_START          CDP1864_SCANLINE_DISPLAY_START - 2
#define CDP1864_SCANLINE_INT_END            CDP1864_SCANLINE_DISPLAY_START
#define CDP1864_SCANLINE_EFX_TOP_START      CDP1864_SCANLINE_DISPLAY_START - 4
#define CDP1864_SCANLINE_EFX_TOP_END        CDP1864_SCANLINE_DISPLAY_START
#define CDP1864_SCANLINE_EFX_BOTTOM_START   CDP1864_SCANLINE_DISPLAY_END - 4
#define CDP1864_SCANLINE_EFX_BOTTOM_END     CDP1864_SCANLINE_DISPLAY_END



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CDP1864_ADD(_tag, _screen_tag, _clock, _inlace, _irq, _dma_out, _efx, _hsync, _rdata, _bdata, _gdata) \
	MCFG_SOUND_ADD(_tag, CDP1864, _clock) \
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
	MCFG_SCREEN_RAW_PARAMS(_clock, CDP1864_SCREEN_WIDTH, CDP1864_HBLANK_END, CDP1864_HBLANK_START, CDP1864_TOTAL_SCANLINES, CDP1864_SCANLINE_VBLANK_END, CDP1864_SCANLINE_VBLANK_START)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cdp1864_device

class cdp1864_device :  public device_t,
						public device_sound_interface,
						public device_video_interface
{
public:
	// construction/destruction
	cdp1864_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	template<class _inlace> void set_inlace_callback(_inlace inlace) { m_read_inlace.set_callback(inlace); }
	template<class _irq> void set_irq_callback(_irq irq) { m_write_irq.set_callback(irq); }
	template<class _dma_out> void set_dma_out_callback(_dma_out dma_out) { m_write_dma_out.set_callback(dma_out); }
	template<class _efx> void set_efx_callback(_efx efx) { m_write_efx.set_callback(efx); }
	template<class _hsync> void set_hsync_callback(_hsync hsync) { m_write_hsync.set_callback(hsync); }
	template<class _rdata> void set_rdata_callback(_rdata rdata) { m_read_rdata.set_callback(rdata); }
	template<class _bdata> void set_bdata_callback(_bdata bdata) { m_read_bdata.set_callback(bdata); }
	template<class _gdata> void set_gdata_callback(_gdata gdata) { m_read_gdata.set_callback(gdata); }
	void set_chrominance_resistors(double r, double b, double g, double bkg) { m_chr_r = r; m_chr_b = b; m_chr_g = g; m_chr_bkg = bkg; }

	DECLARE_READ8_MEMBER( dispon_r );
	DECLARE_READ8_MEMBER( dispoff_r );

	DECLARE_WRITE8_MEMBER( step_bgcolor_w );
	DECLARE_WRITE8_MEMBER( tone_latch_w );

	DECLARE_WRITE8_MEMBER( dma_w );

	DECLARE_WRITE_LINE_MEMBER( con_w );
	DECLARE_WRITE_LINE_MEMBER( aoe_w );
	DECLARE_WRITE_LINE_MEMBER( evs_w );

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

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

	static const int bckgnd[];

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
	INT16 m_signal;                 // current signal
	int m_incr;                     // initial wave state

	// timers
	emu_timer *m_int_timer;
	emu_timer *m_efx_timer;
	emu_timer *m_dma_timer;
	emu_timer *m_hsync_timer;
};


// device type definition
extern const device_type CDP1864;



#endif
