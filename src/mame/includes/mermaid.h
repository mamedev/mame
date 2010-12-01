/*************************************************************************

    Mermaid

*************************************************************************/

class mermaid_state : public driver_device
{
public:
	mermaid_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

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

	/* sound-related */
	UINT32   adpcm_pos;
	UINT32   adpcm_end;
	UINT8    adpcm_idle;
	int      adpcm_data;
	UINT8    adpcm_trigger;
	UINT8    adpcm_rom_sel;
	UINT8    adpcm_play_reg;

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
