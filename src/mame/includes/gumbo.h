/*************************************************************************

    Gumbo - Miss Bingo - Miss Puzzle

*************************************************************************/

class gumbo_state : public driver_device
{
public:
	gumbo_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *    bg_videoram;
	UINT16 *    fg_videoram;
//  UINT16 *    paletteram; // currently this uses generic palette handling

	/* video-related */
	tilemap_t    *bg_tilemap;
	tilemap_t    *fg_tilemap;
};


/*----------- defined in video/gumbo.c -----------*/

WRITE16_HANDLER( gumbo_bg_videoram_w );
WRITE16_HANDLER( gumbo_fg_videoram_w );

VIDEO_START( gumbo );
VIDEO_UPDATE( gumbo );
