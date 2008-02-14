/*----------- defined in video/paradise.c -----------*/

extern UINT8 *paradise_vram_0,*paradise_vram_1,*paradise_vram_2;
extern int paradise_sprite_inc;

WRITE8_HANDLER( paradise_vram_0_w );
WRITE8_HANDLER( paradise_vram_1_w );
WRITE8_HANDLER( paradise_vram_2_w );

WRITE8_HANDLER( paradise_flipscreen_w );
WRITE8_HANDLER( tgtball_flipscreen_w );
WRITE8_HANDLER( paradise_palette_w );
WRITE8_HANDLER( paradise_pixmap_w );

WRITE8_HANDLER( paradise_priority_w );
WRITE8_HANDLER( paradise_palbank_w );

VIDEO_START( paradise );
VIDEO_UPDATE( paradise );
VIDEO_UPDATE( torus );
VIDEO_UPDATE( madball );
