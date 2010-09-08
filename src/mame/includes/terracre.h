class terracre_state : public driver_device
{
public:
	terracre_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *videoram;
};


/*----------- defined in video/terracre.c -----------*/

extern UINT16 *amazon_videoram;

PALETTE_INIT( amazon );
WRITE16_HANDLER( amazon_background_w );
WRITE16_HANDLER( amazon_foreground_w );
WRITE16_HANDLER( amazon_scrolly_w );
WRITE16_HANDLER( amazon_scrollx_w );
WRITE16_HANDLER( amazon_flipscreen_w );
VIDEO_START( amazon );
VIDEO_UPDATE( amazon );
