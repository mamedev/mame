/***************************************************************************

    1942

***************************************************************************/

class _1942_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, _1942_state(machine)); }

	_1942_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT8 * fg_videoram;
	UINT8 * bg_videoram;
	UINT8 * spriteram;
	size_t  spriteram_size;

	/* video-related */
	tilemap_t *fg_tilemap, *bg_tilemap;
	int palette_bank;
	UINT8 scroll[2];

	/* devices */
	running_device *audiocpu;
};



/*----------- defined in video/1942.c -----------*/

extern WRITE8_HANDLER( c1942_fgvideoram_w );
extern WRITE8_HANDLER( c1942_bgvideoram_w );
extern WRITE8_HANDLER( c1942_scroll_w );
extern WRITE8_HANDLER( c1942_c804_w );
extern WRITE8_HANDLER( c1942_palette_bank_w );

extern PALETTE_INIT( 1942 );
extern VIDEO_START( 1942 );
extern VIDEO_UPDATE( 1942 );
