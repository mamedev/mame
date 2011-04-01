class targeth_state : public driver_device
{
public:
	targeth_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *m_vregs;
	UINT16 *m_videoram;
	UINT16 *m_spriteram;
	tilemap_t *m_pant[2];
};


/*----------- defined in video/targeth.c -----------*/

WRITE16_HANDLER( targeth_vram_w );
VIDEO_START( targeth );
SCREEN_UPDATE( targeth );
