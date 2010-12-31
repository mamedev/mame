/*************************************************************************

    King of Boxer - Ring King

*************************************************************************/

class kingofb_state : public driver_device
{
public:
	kingofb_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    videoram2;
	UINT8 *    colorram;
	UINT8 *    colorram2;
	UINT8 *    spriteram;
	UINT8 *    scroll_y;
	size_t     spriteram_size;

	/* video-related */
	tilemap_t    *bg_tilemap, *fg_tilemap;
	int        palette_bank;

	/* misc */
	int        nmi_enable;

	/* devices */
	device_t *video_cpu;
	device_t *sprite_cpu;
	device_t *audio_cpu;
};


/*----------- defined in video/kingobox.c -----------*/

WRITE8_HANDLER( kingofb_videoram_w );
WRITE8_HANDLER( kingofb_colorram_w );
WRITE8_HANDLER( kingofb_videoram2_w );
WRITE8_HANDLER( kingofb_colorram2_w );
WRITE8_HANDLER( kingofb_f800_w );

PALETTE_INIT( kingofb );
VIDEO_START( kingofb );
VIDEO_UPDATE( kingofb );

PALETTE_INIT( ringking );
VIDEO_START( ringking );
VIDEO_UPDATE( ringking );
