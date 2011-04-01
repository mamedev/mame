class cabal_state : public driver_device
{
public:
	cabal_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *m_spriteram;
	UINT16 *m_colorram;
	UINT16 *m_videoram;
	size_t m_spriteram_size;
	tilemap_t *m_background_layer;
	tilemap_t *m_text_layer;
	int m_sound_command1;
	int m_sound_command2;
	int m_last[4];
};


/*----------- defined in video/cabal.c -----------*/

extern VIDEO_START( cabal );
extern SCREEN_UPDATE( cabal );
WRITE16_HANDLER( cabal_flipscreen_w );
WRITE16_HANDLER( cabal_background_videoram16_w );
WRITE16_HANDLER( cabal_text_videoram16_w );
