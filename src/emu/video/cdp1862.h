/**********************************************************************

    RCA CDP1862 COS/MOS Color Generator Controller emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                    RD   1 |*    \_/     | 24  Vdd
                _RESET   2 |             | 23  R LUM
                  _CON   3 |             | 22  G LUM
                 B CHR   4 |             | 21  GD
                 B LUM   5 |             | 20  BKG LUM
                   BKG   6 |   CDP1862   | 19  G CHR
               _LD CLK   7 |             | 18  R CHR
                   STP   8 |             | 17  BKG CHR
               CLK OUT   9 |             | 16  BD
                 _SYNC  10 |             | 15  BURST
                LUM IN  11 |             | 14  _XTAL
                   Vss  12 |_____________| 13  XTAL

**********************************************************************/

#pragma once

#ifndef __CDP1862__
#define __CDP1862__

#include "emu.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define CPD1862_CLOCK   XTAL_7_15909MHz



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CDP1862_ADD(_tag, _screen_tag, _clock, _rd, _bd, _gd) \
	MCFG_DEVICE_ADD(_tag, CDP1862, _clock) \
	MCFG_VIDEO_SET_SCREEN(_screen_tag) \
	downcast<cdp1862_device *>(device)->set_rd_callback(DEVCB2_##_rd); \
	downcast<cdp1862_device *>(device)->set_bd_callback(DEVCB2_##_bd); \
	downcast<cdp1862_device *>(device)->set_gd_callback(DEVCB2_##_gd);

#define MCFG_CDP1862_LUMINANCE(_r, _b, _g, _bkg) \
	downcast<cdp1862_device *>(device)->set_luminance_resistors(_r, _b, _g, _bkg);

#define MCFG_CDP1862_CHROMINANCE(_r, _b, _g, _bkg) \
	downcast<cdp1862_device *>(device)->set_chrominance_resistors(_r, _b, _g, _bkg);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cdp1862_device

class cdp1862_device :  public device_t,
						public device_video_interface
{
public:
	// construction/destruction
	cdp1862_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _rd> void set_rd_callback(_rd rd) { m_read_rd.set_callback(rd); }
	template<class _bd> void set_bd_callback(_bd bd) { m_read_bd.set_callback(bd); }
	template<class _gd> void set_gd_callback(_gd gd) { m_read_gd.set_callback(gd); }
	void set_luminance_resistors(double r, double b, double g, double bkg) { m_lum_r = r; m_lum_b = b; m_lum_g = g; m_lum_bkg = bkg; }
	void set_chrominance_resistors(double r, double b, double g, double bkg) { m_chr_r = r; m_chr_b = b; m_chr_g = g; m_chr_bkg = bkg; }

	DECLARE_WRITE8_MEMBER( dma_w );
	DECLARE_WRITE_LINE_MEMBER( bkg_w );
	DECLARE_WRITE_LINE_MEMBER( con_w );

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:
	inline void initialize_palette();

	devcb2_read_line m_read_rd;
	devcb2_read_line m_read_bd;
	devcb2_read_line m_read_gd;

	bitmap_rgb32 m_bitmap;          // bitmap

	double m_lum_r;             // red luminance resistor value
	double m_lum_b;             // blue luminance resistor value
	double m_lum_g;             // green luminance resistor value
	double m_lum_bkg;           // background luminance resistor value

	double m_chr_r;             // red chrominance resistor value
	double m_chr_b;             // blue chrominance resistor value
	double m_chr_g;             // green chrominance resistor value
	double m_chr_bkg;           // background chrominance resistor value

	rgb_t m_palette[16];
	int m_bgcolor;                  // background color
	int m_con;                      // color on
};


// device type definition
extern const device_type CDP1862;



#endif
