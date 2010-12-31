/*************************************************************************

    Last Duel

*************************************************************************/

class lastduel_state : public driver_device
{
public:
	lastduel_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *    vram;
	UINT16 *    scroll1;
	UINT16 *    scroll2;
//  UINT16 *    spriteram;  // this currently uses generic buffered spriteram
	UINT16 *    paletteram;

	/* video-related */
	tilemap_t     *bg_tilemap, *fg_tilemap, *tx_tilemap;
	UINT16      scroll[8];
	int         sprite_flipy_mask, sprite_pri_mask, tilemap_priority;

	/* devices */
	device_t *audiocpu;
};

/*----------- defined in video/lastduel.c -----------*/

WRITE16_HANDLER( lastduel_vram_w );
WRITE16_HANDLER( lastduel_flip_w );
WRITE16_HANDLER( lastduel_scroll1_w );
WRITE16_HANDLER( lastduel_scroll2_w );
WRITE16_HANDLER( madgear_scroll1_w );
WRITE16_HANDLER( madgear_scroll2_w );
WRITE16_HANDLER( lastduel_scroll_w );
WRITE16_HANDLER( lastduel_palette_word_w );

VIDEO_START( lastduel );
VIDEO_START( madgear );
VIDEO_UPDATE( lastduel );
VIDEO_UPDATE( madgear );
VIDEO_EOF( lastduel );
