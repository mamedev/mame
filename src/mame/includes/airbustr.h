/*************************************************************************

    Air Buster

*************************************************************************/

class airbustr_state : public driver_device
{
public:
	airbustr_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    videoram2;
	UINT8 *    colorram;
	UINT8 *    colorram2;
	UINT8 *    paletteram;
	UINT8 *    devram;

	/* video-related */
	tilemap_t    *bg_tilemap, *fg_tilemap;
	bitmap_t   *sprites_bitmap;
	int        bg_scrollx, bg_scrolly, fg_scrollx, fg_scrolly, highbits;

	/* misc */
	int        soundlatch_status, soundlatch2_status;
	int        master_addr;
	int        slave_addr;

	/* devices */
	running_device *master;
	running_device *slave;
	running_device *audiocpu;
	running_device *pandora;
};


/*----------- defined in video/airbustr.c -----------*/

WRITE8_HANDLER( airbustr_videoram_w );
WRITE8_HANDLER( airbustr_colorram_w );
WRITE8_HANDLER( airbustr_videoram2_w );
WRITE8_HANDLER( airbustr_colorram2_w );
WRITE8_HANDLER( airbustr_scrollregs_w );

VIDEO_START( airbustr );
VIDEO_UPDATE( airbustr );
VIDEO_EOF( airbustr );
