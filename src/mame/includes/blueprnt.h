/***************************************************************************

    Blue Print

***************************************************************************/

class blueprnt_state : public driver_device
{
public:
	blueprnt_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 * videoram;
	UINT8 * colorram;
	UINT8 * spriteram;
	UINT8 * scrollram;
	size_t  spriteram_size;

	/* video-related */
	tilemap_t *bg_tilemap;
	int     gfx_bank;

	/* misc */
	int     dipsw;

	/* devices */
	running_device *audiocpu;
};


/*----------- defined in video/blueprnt.c -----------*/

WRITE8_HANDLER( blueprnt_videoram_w );
WRITE8_HANDLER( blueprnt_colorram_w );
WRITE8_HANDLER( blueprnt_flipscreen_w );

PALETTE_INIT( blueprnt );
VIDEO_START( blueprnt );
VIDEO_UPDATE( blueprnt );
