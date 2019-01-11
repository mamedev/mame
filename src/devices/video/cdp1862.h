// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA CDP1862 COS/MOS Color Generator Controller emulation

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

#ifndef MAME_VIDEO_CDP1862_H
#define MAME_VIDEO_CDP1862_H

#pragma once




//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define CPD1862_CLOCK   XTAL(7'159'090)



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CDP1861_RD_CALLBACK(_read) \
	devcb = &downcast<cdp1862_device &>(*device).set_rd_rd_callback(DEVCB_##_read);

#define MCFG_CDP1861_BD_CALLBACK(_read) \
	devcb = &downcast<cdp1862_device &>(*device).set_bd_rd_callback(DEVCB_##_read);

#define MCFG_CDP1861_GD_CALLBACK(_read) \
	devcb = &downcast<cdp1862_device &>(*device).set_gd_rd_callback(DEVCB_##_read);

#define MCFG_CDP1862_LUMINANCE(_r, _b, _g, _bkg) \
	downcast<cdp1862_device &>(*device).set_luminance(_r, _b, _g, _bkg);

#define MCFG_CDP1862_CHROMINANCE(_r, _b, _g, _bkg) \
	downcast<cdp1862_device &>(*device).set_chrominance(_r, _b, _g, _bkg);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cdp1862_device

class cdp1862_device :  public device_t,
						public device_video_interface
{
public:
	// construction/destruction
	cdp1862_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> devcb_base &set_rd_rd_callback(Object &&cb) { return m_read_rd.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_bd_rd_callback(Object &&cb) { return m_read_bd.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_gd_rd_callback(Object &&cb) { return m_read_gd.set_callback(std::forward<Object>(cb)); }

	void set_luminance(double r, double b, double g, double bkg) { m_lum_r = r; m_lum_b = b; m_lum_g = g; m_lum_bkg = bkg; }
	void set_chrominance(double r, double b, double g, double bkg) { m_chr_r = r; m_chr_b = b; m_chr_g = g; m_chr_bkg = bkg; }

	DECLARE_WRITE8_MEMBER( dma_w );
	DECLARE_WRITE_LINE_MEMBER( bkg_w );
	DECLARE_WRITE_LINE_MEMBER( con_w );

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	inline void initialize_palette();

	devcb_read_line m_read_rd;
	devcb_read_line m_read_bd;
	devcb_read_line m_read_gd;

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
DECLARE_DEVICE_TYPE(CDP1862, cdp1862_device)

#endif // MAME_VIDEO_CDP1862_H
