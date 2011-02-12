class deco_mlc_state : public driver_device
{
public:
	deco_mlc_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT32 *mlc_ram;
	UINT32 *irq_ram;
	timer_device *raster_irq_timer;
	int mainCpuIsArm;
	int mlc_raster_table[9][256];
	UINT32 vbl_i;
	int lastScanline[9];
	UINT32 *mlc_vram;
	UINT32 *mlc_clip_ram;
	UINT32 colour_mask;
	UINT32 *mlc_buffered_spriteram;
};


/*----------- defined in video/deco_mlc.c -----------*/

VIDEO_START( mlc );
VIDEO_UPDATE( mlc );
VIDEO_EOF( mlc );
