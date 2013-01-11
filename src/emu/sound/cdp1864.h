/**********************************************************************

    RCA CDP1864C COS/MOS PAL Compatible Color TV Interface

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

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

#define MCFG_CDP1864_ADD(_tag, _clock, _config) \
	MCFG_SOUND_ADD(_tag, CDP1864, _clock) \
	MCFG_DEVICE_CONFIG(_config)


#define MCFG_CDP1864_SCREEN_ADD(_tag, _clock) \
	MCFG_SCREEN_ADD(_tag, RASTER) \
	MCFG_SCREEN_RAW_PARAMS(_clock, CDP1864_SCREEN_WIDTH, CDP1864_HBLANK_END, CDP1864_HBLANK_START, CDP1864_TOTAL_SCANLINES, CDP1864_SCANLINE_VBLANK_END, CDP1864_SCANLINE_VBLANK_START)


#define CDP1864_INTERFACE(name) \
	const cdp1864_interface (name) =


#define CDP1864_NON_INTERLACED \
	DEVCB_LINE_VCC


#define CDP1864_INTERLACED \
	DEVCB_LINE_GND



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cdp1864_interface

struct cdp1864_interface
{
	const char *m_cpu_tag;
	const char *m_screen_tag;

	devcb_read_line             m_in_inlace_cb;

	devcb_read_line             m_in_rdata_cb;
	devcb_read_line             m_in_bdata_cb;
	devcb_read_line             m_in_gdata_cb;

	devcb_write_line            m_out_int_cb;
	devcb_write_line            m_out_dmao_cb;
	devcb_write_line            m_out_efx_cb;
	devcb_write_line            m_out_hsync_cb;

	double m_res_r;             // red output resistor value
	double m_res_g;             // green output resistor value
	double m_res_b;             // blue output resistor value
	double m_res_bkg;           // background output resistor value
};



// ======================> cdp1864_device

class cdp1864_device :  public device_t,
						public device_sound_interface,
						public cdp1864_interface
{
public:
	// construction/destruction
	cdp1864_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

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
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// internal callbacks
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

private:
	inline void initialize_palette();

	static const device_timer_id TIMER_INT = 0;
	static const device_timer_id TIMER_EFX = 1;
	static const device_timer_id TIMER_DMA = 2;
	static const device_timer_id TIMER_HSYNC = 3;

	devcb_resolved_read_line        m_in_inlace_func;
	devcb_resolved_read_line        m_in_rdata_func;
	devcb_resolved_read_line        m_in_bdata_func;
	devcb_resolved_read_line        m_in_gdata_func;
	devcb_resolved_write_line       m_out_int_func;
	devcb_resolved_write_line       m_out_dmao_func;
	devcb_resolved_write_line       m_out_efx_func;
	devcb_resolved_write_line       m_out_hsync_func;

	cpu_device *m_cpu;
	screen_device *m_screen;        // screen
	bitmap_rgb32 m_bitmap;          // bitmap
	sound_stream *m_stream;         // sound output

	// video state
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
