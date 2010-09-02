class spbactn_state : public driver_device
{
public:
	spbactn_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *bgvideoram;
	UINT16 *fgvideoram;
	UINT16 *spvideoram;

	bitmap_t *tile_bitmap_bg;
	bitmap_t *tile_bitmap_fg;
};


/*----------- defined in video/spbactn.c -----------*/

VIDEO_START( spbactn );
VIDEO_UPDATE( spbactn );
