/**********************************************************************

    RCA CDP1864C COS/MOS PAL Compatible Color TV Interface

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            ________________
            INLACE   1  ---|       \/       |---  40  Vdd
           CLK IN_   2  ---|                |---  39  AUD
          CLR OUT_   3  ---|                |---  38  CLR IN_
               AOE   4  ---|                |---  37  DMA0_
               SC1   5  ---|                |---  36  INT_
               SC0   6  ---|                |---  35  TPA
              MRD_   7  ---|                |---  34  TPB
             BUS 7   8  ---|                |---  33  EVS
             BUS 6   9  ---|                |---  32  V SYNC
             BUS 5  10  ---|    CDP1864C    |---  31  H SYNC
             BUS 4  11  ---|    top view    |---  30  C SYNC_
             BUS 3  12  ---|                |---  29  RED
             BUS 2  13  ---|                |---  28  BLUE
             BUS 1  14  ---|                |---  27  GREEN
             BUS 0  15  ---|                |---  26  BCK GND_
              CON_  16  ---|                |---  25  BURST
                N2  17  ---|                |---  24  ALT
               EF_  18  ---|                |---  23  R DATA
                N0  19  ---|                |---  22  G DATA
               Vss  20  ---|________________|---  21  B DATA


           http://homepage.mac.com/ruske/cosmacelf/cdp1864.pdf

**********************************************************************/

#ifndef __CDP1864__
#define __CDP1864__

#include "devlegcy.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

#define CDP1864_CLOCK	XTAL_1_75MHz

#define CDP1864_VISIBLE_COLUMNS	64
#define CDP1864_VISIBLE_LINES	192

#define CDP1864_HBLANK_END		 1 * 8
#define CDP1864_HBLANK_START	13 * 8
#define CDP1864_HSYNC_START		 0 * 8
#define CDP1864_HSYNC_END		 1 * 8
#define CDP1864_SCREEN_START	 4 * 8
#define CDP1864_SCREEN_END		12 * 8
#define CDP1864_SCREEN_WIDTH	14 * 8

#define CDP1864_TOTAL_SCANLINES				312

#define CDP1864_SCANLINE_VBLANK_START		CDP1864_TOTAL_SCANLINES - 4
#define CDP1864_SCANLINE_VBLANK_END			20
#define CDP1864_SCANLINE_VSYNC_START		0
#define CDP1864_SCANLINE_VSYNC_END			4
#define CDP1864_SCANLINE_DISPLAY_START		60 // ???
#define CDP1864_SCANLINE_DISPLAY_END		CDP1864_SCANLINE_DISPLAY_START + CDP1864_VISIBLE_LINES
#define CDP1864_SCANLINE_INT_START			CDP1864_SCANLINE_DISPLAY_START - 2
#define CDP1864_SCANLINE_INT_END			CDP1864_SCANLINE_DISPLAY_START
#define CDP1864_SCANLINE_EFX_TOP_START		CDP1864_SCANLINE_DISPLAY_START - 4
#define CDP1864_SCANLINE_EFX_TOP_END		CDP1864_SCANLINE_DISPLAY_START
#define CDP1864_SCANLINE_EFX_BOTTOM_START	CDP1864_SCANLINE_DISPLAY_END - 4
#define CDP1864_SCANLINE_EFX_BOTTOM_END		CDP1864_SCANLINE_DISPLAY_END

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

DECLARE_LEGACY_SOUND_DEVICE(CDP1864, cdp1864);

#define MCFG_CDP1864_ADD(_tag, _clock, _config) \
	MCFG_SOUND_ADD(_tag, CDP1864, _clock) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_CDP1864_SCREEN_ADD(_tag, _clock) \
	MCFG_SCREEN_ADD(_tag, RASTER) \
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16) \
	MCFG_SCREEN_RAW_PARAMS(_clock, CDP1864_SCREEN_WIDTH, CDP1864_HBLANK_END, CDP1864_HBLANK_START, CDP1864_TOTAL_SCANLINES, CDP1864_SCANLINE_VBLANK_END, CDP1864_SCANLINE_VBLANK_START)

#define CDP1864_INTERFACE(name) \
	const cdp1864_interface (name) =

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

enum _cdp1864_format {
	CDP1864_NON_INTERLACED = 0,
	CDP1864_INTERLACED
};
typedef enum _cdp1864_format cdp1864_format;

typedef struct _cdp1864_interface cdp1864_interface;
struct _cdp1864_interface
{
	const char *cpu_tag;		/* cpu we are working with */
	const char *screen_tag;		/* screen we are acting on */

	cdp1864_format interlace;	/* interlace */

	devcb_read_line				in_rdata_func;
	devcb_read_line				in_bdata_func;
	devcb_read_line				in_gdata_func;

	/* this gets called for every change of the INT pin (pin 36) */
	devcb_write_line			out_int_func;

	/* this gets called for every change of the DMAO pin (pin 37) */
	devcb_write_line			out_dmao_func;

	/* this gets called for every change of the EFX pin (pin 18) */
	devcb_write_line			out_efx_func;

	double res_r;				/* red output resistor value */
	double res_g;				/* green output resistor value */
	double res_b;				/* blue output resistor value */
	double res_bkg;				/* background output resistor value */
};

/***************************************************************************
    PROTOTYPES
***************************************************************************/

/* display on (0x69) */
READ8_DEVICE_HANDLER( cdp1864_dispon_r ) ATTR_NONNULL(1);

/* display off (0x6c) */
READ8_DEVICE_HANDLER( cdp1864_dispoff_r ) ATTR_NONNULL(1);

/* step background color (0x61) */
WRITE8_DEVICE_HANDLER( cdp1864_step_bgcolor_w ) ATTR_NONNULL(1);

/* color on */
WRITE_LINE_DEVICE_HANDLER( cdp1864_con_w ) ATTR_NONNULL(1);

/* load tone latch (0x64) */
WRITE8_DEVICE_HANDLER( cdp1864_tone_latch_w ) ATTR_NONNULL(1);

/* audio output enable */
WRITE_LINE_DEVICE_HANDLER( cdp1864_aoe_w ) ATTR_NONNULL(1);

/* DMA write */
WRITE8_DEVICE_HANDLER( cdp1864_dma_w ) ATTR_NONNULL(1);

/* screen update */
void cdp1864_update(device_t *device, bitmap_t *bitmap, const rectangle *cliprect) ATTR_NONNULL(1) ATTR_NONNULL(2) ATTR_NONNULL(3);

#endif
