class bigstrkb_state : public driver_device
{
public:
	bigstrkb_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	tilemap_t *m_tilemap;
	tilemap_t *m_tilemap2;
	tilemap_t *m_tilemap3;

	UINT16 *m_videoram;
	UINT16 *m_videoram2;
	UINT16 *m_videoram3;

	UINT16 *m_vidreg1;
	UINT16 *m_vidreg2;
	UINT16 *m_spriteram;
};


/*----------- defined in video/bigstrkb.c -----------*/

WRITE16_HANDLER( bsb_videoram_w );
WRITE16_HANDLER( bsb_videoram2_w );
WRITE16_HANDLER( bsb_videoram3_w );
VIDEO_START(bigstrkb);
SCREEN_UPDATE(bigstrkb);
