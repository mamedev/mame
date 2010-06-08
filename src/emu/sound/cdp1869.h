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

#ifndef __CDP1869__
#define __CDP1869__

#include "devlegcy.h"


/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

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
#define CDP1869_HBLANK_END			( 5 * CDP1869_CHAR_WIDTH)
#define CDP1869_SCREEN_START_PAL	( 9 * CDP1869_CHAR_WIDTH)
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

#define	CDP1869_PALETTE_LENGTH	8+64

DECLARE_LEGACY_SOUND_DEVICE(CDP1869, cdp1869);

#define MDRV_CDP1869_ADD(_tag, _pixclock, _config) \
	MDRV_DEVICE_ADD(_tag, SOUND_CDP1869, _pixclock) \
	MDRV_DEVICE_CONFIG(_config)

#define MDRV_CDP1869_SCREEN_PAL_ADD(_tag, _clock) \
	MDRV_SCREEN_ADD(_tag, RASTER) \
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16) \
	MDRV_SCREEN_RAW_PARAMS(_clock, CDP1869_SCREEN_WIDTH, CDP1869_HBLANK_END, CDP1869_HBLANK_START, CDP1869_TOTAL_SCANLINES_PAL, CDP1869_SCANLINE_VBLANK_END_PAL, CDP1869_SCANLINE_VBLANK_START_PAL) \
	MDRV_PALETTE_LENGTH(8+64) \
	MDRV_PALETTE_INIT(cdp1869)

#define MDRV_CDP1869_SCREEN_NTSC_ADD(_tag, _clock) \
	MDRV_SCREEN_ADD(_tag, RASTER) \
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16) \
	MDRV_SCREEN_RAW_PARAMS(_clock, CDP1869_SCREEN_WIDTH, CDP1869_HBLANK_END, CDP1869_HBLANK_START, CDP1869_TOTAL_SCANLINES_NTSC, CDP1869_SCANLINE_VBLANK_END_NTSC, CDP1869_SCANLINE_VBLANK_START_NTSC) \
	MDRV_PALETTE_LENGTH(8+64) \
	MDRV_PALETTE_INIT(cdp1869)

#define CDP1869_INTERFACE(_name) \
	const cdp1869_interface (_name) =

#define CDP1869_CHAR_RAM_READ(name) UINT8 name(running_device *device, UINT16 pma, UINT8 cma)
#define CDP1869_CHAR_RAM_WRITE(name) void name(running_device *device, UINT16 pma, UINT8 cma, UINT8 data)
#define CDP1869_PCB_READ(name) int name(running_device *device, UINT16 pma, UINT8 cma)

#define CDP1869_PAL \
	DEVCB_LINE_VCC

#define CDP1869_NTSC \
	DEVCB_LINE_GND

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef UINT8 (*cdp1869_char_ram_read_func)(running_device *device, UINT16 pma, UINT8 cma);
typedef void (*cdp1869_char_ram_write_func)(running_device *device, UINT16 pma, UINT8 cma, UINT8 data);
typedef int (*cdp1869_pcb_read_func)(running_device *device, UINT16 pma, UINT8 cma);

/* interface */
typedef struct _cdp1869_interface cdp1869_interface;
struct _cdp1869_interface
{
	const char *cpu_tag;		/* CPU we work together with */
	const char *screen_tag;		/* screen we are acting on */

	/* pixel clock of the chip is the device clock */
	int color_clock;			/* the chroma clock of the chip */

	/* screen format */
	devcb_read_line					in_pal_ntsc_func;

	/* page memory read function */
	devcb_read8						in_page_ram_func;

	/* page memory write function */
	devcb_write8					out_page_ram_func;

	/* page memory color bit read function */
	cdp1869_pcb_read_func			pcb_r;

	/* character memory read function */
	cdp1869_char_ram_read_func		char_ram_r;

	/* character memory write function */
	cdp1869_char_ram_write_func		char_ram_w;

	/* if specified, this gets called for every change of the predisplay pin (CDP1870/76 pin 1) */
	devcb_write_line				out_prd_func;
};

/***************************************************************************
    PROTOTYPES
***************************************************************************/

/* palette initialization */
PALETTE_INIT( cdp1869 );

/* register access */
WRITE8_DEVICE_HANDLER( cdp1869_out3_w );
WRITE8_DEVICE_HANDLER( cdp1869_out4_w );
WRITE8_DEVICE_HANDLER( cdp1869_out5_w );
WRITE8_DEVICE_HANDLER( cdp1869_out6_w );
WRITE8_DEVICE_HANDLER( cdp1869_out7_w );

/* character memory access */
READ8_DEVICE_HANDLER ( cdp1869_charram_r );
WRITE8_DEVICE_HANDLER ( cdp1869_charram_w );

/* page memory access */
READ8_DEVICE_HANDLER ( cdp1869_pageram_r );
WRITE8_DEVICE_HANDLER ( cdp1869_pageram_w );

/* predisplay */
READ_LINE_DEVICE_HANDLER( cdp1869_predisplay_r );

/* screen update */
void cdp1869_update(running_device *device, bitmap_t *bitmap, const rectangle *cliprect);

#endif
