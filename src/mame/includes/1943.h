/***************************************************************************

    1943

***************************************************************************/

class _1943_state : public driver_device
{
public:
	_1943_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 * videoram;
	UINT8 * colorram;
	UINT8 * spriteram;
	UINT8 * scrollx;
	UINT8 * scrolly;
	UINT8 * bgscrollx;
	size_t  spriteram_size;

	/* video-related */
	tilemap_t *fg_tilemap, *bg_tilemap, *bg2_tilemap;
	int     char_on, obj_on, bg1_on, bg2_on;
};



/*----------- defined in video/1943.c -----------*/

extern WRITE8_HANDLER( c1943_c804_w );
extern WRITE8_HANDLER( c1943_d806_w );
extern WRITE8_HANDLER( c1943_videoram_w );
extern WRITE8_HANDLER( c1943_colorram_w );

extern PALETTE_INIT( 1943 );
extern VIDEO_START( 1943 );
extern VIDEO_UPDATE( 1943 );
