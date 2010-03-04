class mappy_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, mappy_state(machine)); }

	mappy_state(running_machine &machine) { }

	UINT8 *videoram;
	UINT8 *spriteram;
	tilemap_t *bg_tilemap;
	bitmap_t *sprite_bitmap;

	UINT8 scroll;
	int mux;
};


/*----------- defined in video/mappy.c -----------*/

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
void mappy_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT8 *spriteram, int xoffs, int yoffs, int transcolor);
