class cabal_state : public driver_device
{
public:
	cabal_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *spriteram;
	UINT16 *colorram;
	UINT16 *videoram;
	size_t spriteram_size;
	tilemap_t *background_layer;
	tilemap_t *text_layer;
	int sound_command1;
	int sound_command2;
	int last[4];
};


/*----------- defined in video/cabal.c -----------*/

extern VIDEO_START( cabal );
extern VIDEO_UPDATE( cabal );
WRITE16_HANDLER( cabal_flipscreen_w );
WRITE16_HANDLER( cabal_background_videoram16_w );
WRITE16_HANDLER( cabal_text_videoram16_w );
