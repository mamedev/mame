class terracre_state : public driver_device
{
public:
	terracre_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *videoram;
	const UINT16 *mpProtData;
	UINT8 mAmazonProtCmd;
	UINT8 mAmazonProtReg[6];
	UINT16 *amazon_videoram;
	UINT16 xscroll;
	UINT16 yscroll;
	tilemap_t *background;
	tilemap_t *foreground;
	UINT16 *spriteram;
};


/*----------- defined in video/terracre.c -----------*/

PALETTE_INIT( amazon );
WRITE16_HANDLER( amazon_background_w );
WRITE16_HANDLER( amazon_foreground_w );
WRITE16_HANDLER( amazon_scrolly_w );
WRITE16_HANDLER( amazon_scrollx_w );
WRITE16_HANDLER( amazon_flipscreen_w );
VIDEO_START( amazon );
SCREEN_UPDATE( amazon );
