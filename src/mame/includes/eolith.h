/*----------- defined in video/eolith.c -----------*/

extern int eolith_buffer;

READ32_HANDLER( eolith_vram_r );
WRITE32_HANDLER( eolith_vram_w );
VIDEO_START( eolith );
VIDEO_UPDATE( eolith );
