/*************************************************************************

    Atari "Stella on Steroids" hardware

*************************************************************************/

/*----------- defined in video/beathead.c -----------*/

extern UINT32 *	beathead_vram_bulk_latch;
extern UINT32 *	beathead_palette_select;

VIDEO_START( beathead );
VIDEO_UPDATE( beathead );

void beathead_scanline_update(int scanline);

WRITE32_HANDLER( beathead_vram_transparent_w );
WRITE32_HANDLER( beathead_vram_bulk_w );
WRITE32_HANDLER( beathead_vram_latch_w );
WRITE32_HANDLER( beathead_vram_copy_w );
WRITE32_HANDLER( beathead_finescroll_w );
WRITE32_HANDLER( beathead_palette_w );
READ32_HANDLER( beathead_hsync_ram_r );
WRITE32_HANDLER( beathead_hsync_ram_w );
