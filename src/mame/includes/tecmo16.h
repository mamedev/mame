class tecmo16_state : public driver_device
{
public:
	tecmo16_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *videoram;
	UINT16 *colorram;
	UINT16 *videoram2;
	UINT16 *colorram2;
	UINT16 *charram;
	tilemap_t *fg_tilemap;
	tilemap_t *bg_tilemap;
	tilemap_t *tx_tilemap;
	bitmap_t *sprite_bitmap;
	bitmap_t *tile_bitmap_bg;
	bitmap_t *tile_bitmap_fg;
	int flipscreen;
	int game_is_riot;
	UINT16 scroll_x_w;
	UINT16 scroll_y_w;
	UINT16 scroll2_x_w;
	UINT16 scroll2_y_w;
	UINT16 scroll_char_x_w;
	UINT16 scroll_char_y_w;
	UINT16 *spriteram;
	size_t spriteram_size;
};


/*----------- defined in video/tecmo16.c -----------*/

WRITE16_HANDLER( tecmo16_videoram_w );
WRITE16_HANDLER( tecmo16_colorram_w );
WRITE16_HANDLER( tecmo16_videoram2_w );
WRITE16_HANDLER( tecmo16_colorram2_w );
WRITE16_HANDLER( tecmo16_charram_w );
WRITE16_HANDLER( tecmo16_flipscreen_w );

WRITE16_HANDLER( tecmo16_scroll_x_w );
WRITE16_HANDLER( tecmo16_scroll_y_w );
WRITE16_HANDLER( tecmo16_scroll2_x_w );
WRITE16_HANDLER( tecmo16_scroll2_y_w );
WRITE16_HANDLER( tecmo16_scroll_char_x_w );
WRITE16_HANDLER( tecmo16_scroll_char_y_w );

VIDEO_START( fstarfrc );
VIDEO_START( ginkun );
VIDEO_START( riot );
SCREEN_UPDATE( tecmo16 );
