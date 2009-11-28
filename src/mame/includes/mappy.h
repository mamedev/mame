/*----------- defined in video/mappy.c -----------*/

extern UINT8 *mappy_videoram;
extern UINT8 *mappy_spriteram;

VIDEO_START( phozon );
PALETTE_INIT( phozon );
VIDEO_UPDATE( phozon );

PALETTE_INIT( superpac );
PALETTE_INIT( mappy );
VIDEO_START( superpac );
VIDEO_START( mappy );
VIDEO_UPDATE( superpac );
VIDEO_UPDATE( mappy );
WRITE8_HANDLER( superpac_videoram_w );
WRITE8_HANDLER( mappy_videoram_w );
WRITE8_HANDLER( mappy_scroll_w );
READ8_HANDLER( superpac_flipscreen_r );
WRITE8_HANDLER( superpac_flipscreen_w );
void mappy_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int xoffs, int yoffs, int transcolor);
