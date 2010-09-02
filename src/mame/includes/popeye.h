/*----------- defined in video/popeye.c -----------*/

extern UINT8 *popeye_videoram;
extern UINT8 *popeye_colorram;
extern UINT8 *popeye_background_pos;
extern UINT8 *popeye_palettebank;

WRITE8_HANDLER( popeye_videoram_w );
WRITE8_HANDLER( popeye_colorram_w );
WRITE8_HANDLER( popeye_bitmap_w );
WRITE8_HANDLER( skyskipr_bitmap_w );

PALETTE_INIT( popeye );
PALETTE_INIT( popeyebl );
VIDEO_START( skyskipr );
VIDEO_START( popeye );
VIDEO_UPDATE( popeye );
