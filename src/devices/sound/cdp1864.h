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

	auto inlace_cb() { return m_read_inlace.bind(); }
	auto int_cb() { return m_write_int.bind(); }
	auto dma_out_cb() { return m_write_dma_out.bind(); }
	auto efx_cb() { return m_write_efx.bind(); }
	auto hsync_cb() { return m_write_hsync.bind(); }
	auto rdata_cb() { return m_read_rdata.bind(); }
	auto bdata_cb() { return m_read_bdata.bind(); }
	auto gdata_cb() { return m_read_gdata.bind(); }

	void set_chrominance(double r, double b, double g, double bkg) { m_chr_r = r; m_chr_b = b; m_chr_g = g; m_chr_bkg = bkg; }

	uint8_t dispon_r();
	uint8_t dispoff_r();

	void step_bgcolor_w(uint8_t data);
	void tone_latch_w(uint8_t data);

	void dma_w(uint8_t data);

	void con_w(int state);
	void aoe_w(int state);
	void evs_w(int state);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// internal callbacks
	virtual void sound_stream_update(sound_stream &stream) override;

	TIMER_CALLBACK_MEMBER(int_tick);
	TIMER_CALLBACK_MEMBER(efx_tick);
	TIMER_CALLBACK_MEMBER(dma_tick);

private:
	void initialize_palette();

	static constexpr int bckgnd[4] = { 2, 0, 4, 1 };

	devcb_read_line        m_read_inlace;
	devcb_read_line        m_read_rdata;
	devcb_read_line        m_read_bdata;
	devcb_read_line        m_read_gdata;
	devcb_write_line       m_write_int;
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
	bool m_con;                     // color on

	// sound state
	int m_aoe;                      // audio on
	int m_latch;                    // sound latch
	sound_stream::sample_t m_signal; // current signal
	int m_incr;                     // initial wave state

	// timers
	emu_timer *m_int_timer;
	emu_timer *m_efx_timer;
	emu_timer *m_dma_timer;
};


// device type definition
DECLARE_DEVICE_TYPE(CDP1864, cdp1864_device)

#endif // MAME_DEVICES_SOUND_CDP1864_H
