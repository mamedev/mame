class zodiack_state : public driver_device
{
public:
	zodiack_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *   videoram;
	UINT8 *   attributeram;
	UINT8 *   spriteram;
	UINT8 *   videoram_2;
	UINT8 *   bulletsram;
	size_t    videoram_size;
	size_t    spriteram_size;
	size_t    bulletsram_size;

	/* video-related */
	tilemap_t   *bg_tilemap, *fg_tilemap;

	/* sound-related */
	UINT8     sound_nmi_enabled;

	/* misc */
	int       percuss_hardware;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
};

/*----------- defined in video/zodiack.c -----------*/

WRITE8_HANDLER( zodiack_videoram_w );
WRITE8_HANDLER( zodiack_videoram2_w );
WRITE8_HANDLER( zodiack_attributes_w );
WRITE8_HANDLER( zodiack_flipscreen_w );

PALETTE_INIT( zodiack );
VIDEO_START( zodiack );
VIDEO_UPDATE( zodiack );
