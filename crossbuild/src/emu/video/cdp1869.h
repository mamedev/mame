/*

    RCA CDP1869/70/76 Video Interface System (VIS)

    http://homepage.mac.com/ruske/cosmacelf/cdp1869.pdf

                        ________________                                            ________________
           TPA   1  ---|       \/       |---  40  Vdd          PREDISPLAY_   1  ---|       \/       |---  40  Vdd
           TPB   2  ---|                |---  39  PMSEL          *DISPLAY_   2  ---|                |---  39  PAL/NTSC_
          MRD_   3  ---|                |---  38  PMWR_                PCB   3  ---|                |---  38  CPUCLK
          MWR_   4  ---|                |---  37  *CMSEL              CCB1   4  ---|                |---  37  XTAL (DOT)
         MA0/8   5  ---|                |---  36  CMWR_               BUS7   5  ---|                |---  36  XTAL (DOT)_
         MA1/9   6  ---|                |---  35  PMA0                CCB0   6  ---|                |---  35  *ADDRSTB_
        MA2/10   7  ---|                |---  34  PMA1                BUS6   7  ---|                |---  34  MRD_
        MA3/11   8  ---|                |---  33  PMA2                CDB5   8  ---|                |---  33  TPB
        MA4/12   9  ---|                |---  32  PMA3                BUS5   9  ---|                |---  32  *CMSEL
        MA5/13  10  ---|    CDP1869C    |---  31  PMA4                CDB4  10  ---|  CDP1870/76C   |---  31  BURST
        MA6/14  11  ---|    top view    |---  30  PMA5                BUS4  11  ---|    top view    |---  30  *H SYNC_
        MA7/15  12  ---|                |---  29  PMA6                CDB3  12  ---|                |---  29  COMPSYNC_
            N0  13  ---|                |---  28  PMA7                BUS3  13  ---|                |---  28  LUM / (RED)^
            N1  14  ---|                |---  27  PMA8                CDB2  14  ---|                |---  27  PAL CHROM / (BLUE)^
            N2  15  ---|                |---  26  PMA9                BUS2  15  ---|                |---  26  NTSC CHROM / (GREEN)^
      *H SYNC_  16  ---|                |---  25  CMA3/PMA10          CDB1  16  ---|                |---  25  XTAL_ (CHROM)
     *DISPLAY_  17  ---|                |---  24  CMA2                BUS1  17  ---|                |---  24  XTAL (CHROM)
     *ADDRSTB_  18  ---|                |---  23  CMA1                CDB0  18  ---|                |---  23  EMS_
         SOUND  19  ---|                |---  22  CMA0                BUS0  19  ---|                |---  22  EVS_
           VSS  20  ---|________________|---  21  *N=3_                Vss  20  ---|________________|---  21  *N=3_


                 * = INTERCHIP CONNECTIONS      ^ = FOR THE RGB BOND-OUT OPTION (CDP1876C)      _ = ACTIVE LOW

*/

#ifndef __CDP1869_VIDEO__
#define __CDP1869_VIDEO__

enum {
	CDP1869_NTSC,
	CDP1869_PAL
};

#define CDP1869_DOT_CLK_PAL			5626000.0
#define CDP1869_DOT_CLK_NTSC		5670000.0
#define CDP1869_COLOR_CLK_PAL		8867236.0
#define CDP1869_COLOR_CLK_NTSC		7159090.0

#define CDP1869_CPU_CLK_PAL			(CDP1869_DOT_CLK_PAL / 2)
#define CDP1869_CPU_CLK_NTSC		(CDP1869_DOT_CLK_NTSC / 2)

#define CDP1869_CHAR_WIDTH			6

#define CDP1869_HSYNC_START			(56 * CDP1869_CHAR_WIDTH)
#define CDP1869_HSYNC_END			(60 * CDP1869_CHAR_WIDTH)
#define CDP1869_HBLANK_START		(54 * CDP1869_CHAR_WIDTH)
#define CDP1869_HBLANK_END			(5 * CDP1869_CHAR_WIDTH)
#define CDP1869_SCREEN_START_PAL	(9 * CDP1869_CHAR_WIDTH)
#define CDP1869_SCREEN_START_NTSC	(10 * CDP1869_CHAR_WIDTH)
#define CDP1869_SCREEN_START		(10 * CDP1869_CHAR_WIDTH)
#define CDP1869_SCREEN_END			(50 * CDP1869_CHAR_WIDTH)
#define CDP1869_SCREEN_WIDTH		(60 * CDP1869_CHAR_WIDTH)

#define CDP1869_TOTAL_SCANLINES_PAL				312
#define CDP1869_SCANLINE_VBLANK_START_PAL		304
#define CDP1869_SCANLINE_VBLANK_END_PAL			10
#define CDP1869_SCANLINE_VSYNC_START_PAL		308
#define CDP1869_SCANLINE_VSYNC_END_PAL			312
#define CDP1869_SCANLINE_DISPLAY_START_PAL		44
#define CDP1869_SCANLINE_DISPLAY_END_PAL		260
#define CDP1869_SCANLINE_PREDISPLAY_START_PAL	43
#define CDP1869_SCANLINE_PREDISPLAY_END_PAL		260
#define CDP1869_VISIBLE_SCANLINES_PAL			(CDP1869_SCANLINE_DISPLAY_END_PAL - CDP1869_SCANLINE_DISPLAY_START_PAL)

#define CDP1869_TOTAL_SCANLINES_NTSC			262
#define CDP1869_SCANLINE_VBLANK_START_NTSC		252
#define CDP1869_SCANLINE_VBLANK_END_NTSC		10
#define CDP1869_SCANLINE_VSYNC_START_NTSC		258
#define CDP1869_SCANLINE_VSYNC_END_NTSC			262
#define CDP1869_SCANLINE_DISPLAY_START_NTSC		36
#define CDP1869_SCANLINE_DISPLAY_END_NTSC		228
#define CDP1869_SCANLINE_PREDISPLAY_START_NTSC	35
#define CDP1869_SCANLINE_PREDISPLAY_END_NTSC	228
#define CDP1869_VISIBLE_SCANLINES_NTSC			(CDP1869_SCANLINE_DISPLAY_END_NTSC - CDP1869_SCANLINE_DISPLAY_START_NTSC)

#define CDP1869_FPS_PAL				CDP1869_DOT_CLK_PAL / CDP1869_SCREEN_WIDTH / CDP1869_TOTAL_SCANLINES_PAL
#define CDP1869_FPS_NTSC			CDP1869_DOT_CLK_NTSC / CDP1869_SCREEN_WIDTH / CDP1869_TOTAL_SCANLINES_NTSC

#define CDP1869_WEIGHT_RED		30 // % of max luminance
#define CDP1869_WEIGHT_GREEN	59
#define CDP1869_WEIGHT_BLUE		11

#define CDP1869_CHARRAM_SIZE	0x1000
#define CDP1869_PAGERAM_SIZE	0x800

#define CDP1869_COLUMNS_HALF	20
#define CDP1869_COLUMNS_FULL	40
#define CDP1869_ROWS_HALF		12
#define CDP1869_ROWS_FULL_PAL	25
#define CDP1869_ROWS_FULL_NTSC	24

typedef struct CDP1869_interface
{
	int ntsc_pal;			// display format (NTSC/PAL)
	int charrom_region;		// memory region to load CHARRAM from
	UINT16 charram_size;	// CHARRAM size
	UINT16 pageram_size;	// PAGERAM size
	UINT8 (*get_color_bits)(UINT8 cramdata, UINT16 cramaddr, UINT16 pramaddr); // 0x01 = PCB, 0x02 = CCB0, 0x04 = CCB1
} CDP1869_interface;

WRITE8_HANDLER ( cdp1869_out3_w );
WRITE8_HANDLER ( cdp1869_out4_w );
WRITE8_HANDLER ( cdp1869_out5_w );
WRITE8_HANDLER ( cdp1869_out6_w );
WRITE8_HANDLER ( cdp1869_out7_w );

PALETTE_INIT( cdp1869 );
VIDEO_START( cdp1869 );
VIDEO_UPDATE( cdp1869 );

READ8_HANDLER ( cdp1869_charram_r );
READ8_HANDLER ( cdp1869_pageram_r );
WRITE8_HANDLER ( cdp1869_charram_w );
WRITE8_HANDLER ( cdp1869_pageram_w );

void cdp1869_configure(const CDP1869_interface *intf);

UINT16 cdp1869_get_cma(UINT16 offset);

#endif
