/*************************************************************************

    Mermaid

*************************************************************************/

class mermaid_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, mermaid_state(machine)); }

	mermaid_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    colorram;
	UINT8 *    videoram2;
	UINT8 *    spriteram;
	UINT8 *    bg_scrollram;
	UINT8 *    fg_scrollram;
	UINT8 *    ay8910_enable;
	size_t     spriteram_size;

	/* video-related */
	tilemap_t *bg_tilemap, *fg_tilemap;
	bitmap_t* helper;
	bitmap_t* helper2;
	int coll_bit0, coll_bit1, coll_bit2, coll_bit3, coll_bit6;
	int rougien_gfxbank1, rougien_gfxbank2;

	/* devices */
	running_device *maincpu;
	running_device *ay1;
	running_device *ay2;
};



/*----------- defined in video/mermaid.c -----------*/

WRITE8_HANDLER( mermaid_videoram2_w );
WRITE8_HANDLER( mermaid_videoram_w );
WRITE8_HANDLER( mermaid_colorram_w );
WRITE8_HANDLER( mermaid_flip_screen_x_w );
WRITE8_HANDLER( mermaid_flip_screen_y_w );
WRITE8_HANDLER( mermaid_bg_scroll_w );
WRITE8_HANDLER( mermaid_fg_scroll_w );
WRITE8_HANDLER( rougien_gfxbankswitch1_w );
WRITE8_HANDLER( rougien_gfxbankswitch2_w );
READ8_HANDLER( mermaid_collision_r );

PALETTE_INIT( mermaid );
VIDEO_START( mermaid );
VIDEO_UPDATE( mermaid );
VIDEO_EOF( mermaid );
