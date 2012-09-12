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

#define CPD1862_CLOCK	XTAL_7_15909MHz



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CDP1862_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, CDP1862, _clock) \
	MCFG_DEVICE_CONFIG(_config)


#define CDP1862_INTERFACE(name) \
	const cdp1862_interface (name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> cdp1862_interface

struct cdp1862_interface
{
	const char *m_screen_tag;

	devcb_read_line				m_in_rd_cb;
	devcb_read_line				m_in_bd_cb;
	devcb_read_line				m_in_gd_cb;

	double m_lum_r;				// red luminance resistor value
	double m_lum_b;				// blue luminance resistor value
	double m_lum_g;				// green luminance resistor value
	double m_lum_bkg;			// background luminance resistor value

	double m_chr_r;				// red chrominance resistor value
	double m_chr_b;				// blue chrominance resistor value
	double m_chr_g;				// green chrominance resistor value
	double m_chr_bkg;			// background chrominance resistor value
};



// ======================> cdp1862_device

class cdp1862_device :	public device_t,
                        public cdp1862_interface
{
public:
    // construction/destruction
    cdp1862_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

    DECLARE_WRITE8_MEMBER( dma_w );
	DECLARE_WRITE_LINE_MEMBER( bkg_w );
	DECLARE_WRITE_LINE_MEMBER( con_w );

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
    // device-level overrides
	virtual void device_config_complete();
    virtual void device_start();
    virtual void device_reset();

private:
	inline void initialize_palette();

	devcb_resolved_read_line	m_in_rd_func;
	devcb_resolved_read_line	m_in_bd_func;
	devcb_resolved_read_line	m_in_gd_func;

	screen_device *m_screen;		// screen
	bitmap_rgb32 m_bitmap;			// bitmap

	rgb_t m_palette[16];
	int m_bgcolor;					// background color
	int m_con;						// color on
};


// device type definition
extern const device_type CDP1862;



#endif
