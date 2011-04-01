class darkmist_state : public driver_device
{
public:
	darkmist_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *m_videoram;
	UINT8 *m_workram;
	int m_hw;
	UINT8 *m_scroll;
	UINT8 *m_spritebank;
	tilemap_t *m_bgtilemap;
	tilemap_t *m_fgtilemap;
	tilemap_t *m_txtilemap;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
};


/*----------- defined in video/darkmist.c -----------*/

VIDEO_START( darkmist );
SCREEN_UPDATE( darkmist );
PALETTE_INIT( darkmist );

