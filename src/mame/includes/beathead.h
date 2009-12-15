/*************************************************************************

    Atari "Stella on Steroids" hardware

*************************************************************************/

#include "machine/atarigen.h"

typedef struct _beathead_state beathead_state;
struct _beathead_state
{
	atarigen_state	atarigen;

	UINT32 *		vram_bulk_latch;
	UINT32 *		palette_select;

	UINT32			finescroll;
	offs_t			vram_latch_offset;

	offs_t			hsyncram_offset;
	offs_t			hsyncram_start;
	UINT8			hsyncram[0x800];
};


/*----------- defined in video/beathead.c -----------*/

VIDEO_START( beathead );
VIDEO_UPDATE( beathead );

WRITE32_HANDLER( beathead_vram_transparent_w );
WRITE32_HANDLER( beathead_vram_bulk_w );
WRITE32_HANDLER( beathead_vram_latch_w );
WRITE32_HANDLER( beathead_vram_copy_w );
WRITE32_HANDLER( beathead_finescroll_w );
WRITE32_HANDLER( beathead_palette_w );
READ32_HANDLER( beathead_hsync_ram_r );
WRITE32_HANDLER( beathead_hsync_ram_w );
