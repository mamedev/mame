class spbactn_state : public driver_device
{
public:
	spbactn_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *m_bgvideoram;
	UINT16 *m_fgvideoram;
	UINT16 *m_spvideoram;

	bitmap_t *m_tile_bitmap_bg;
	bitmap_t *m_tile_bitmap_fg;
};


/*----------- defined in video/spbactn.c -----------*/

VIDEO_START( spbactn );
SCREEN_UPDATE( spbactn );
