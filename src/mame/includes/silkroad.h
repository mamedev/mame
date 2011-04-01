class silkroad_state : public driver_device
{
public:
	silkroad_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT32 *m_vidram;
	UINT32 *m_vidram2;
	UINT32 *m_vidram3;
	UINT32 *m_sprram;
	UINT32 *m_regs;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_fg2_tilemap;
	tilemap_t *m_fg3_tilemap;
};


/*----------- defined in video/silkroad.c -----------*/

WRITE32_HANDLER( silkroad_fgram_w );
WRITE32_HANDLER( silkroad_fgram2_w );
WRITE32_HANDLER( silkroad_fgram3_w );
VIDEO_START(silkroad);
SCREEN_UPDATE(silkroad);
