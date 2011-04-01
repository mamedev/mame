class thoop2_state : public driver_device
{
public:
	thoop2_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *m_vregs;
	UINT16 *m_videoram;
	UINT16 *m_spriteram;
	int m_sprite_count[5];
	int *m_sprite_table[5];
	tilemap_t *m_pant[2];
};


/*----------- defined in video/thoop2.c -----------*/

WRITE16_HANDLER( thoop2_vram_w );
VIDEO_START( thoop2 );
SCREEN_UPDATE( thoop2 );
