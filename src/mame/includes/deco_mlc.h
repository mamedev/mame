class deco_mlc_state : public driver_device
{
public:
	deco_mlc_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT32 *m_mlc_ram;
	UINT32 *m_irq_ram;
	timer_device *m_raster_irq_timer;
	int m_mainCpuIsArm;
	int m_mlc_raster_table[9][256];
	UINT32 m_vbl_i;
	int m_lastScanline[9];
	UINT32 *m_mlc_vram;
	UINT32 *m_mlc_clip_ram;
	UINT32 m_colour_mask;
	UINT32 *m_mlc_buffered_spriteram;
	UINT32 *m_spriteram;
	size_t m_spriteram_size;
};


/*----------- defined in video/deco_mlc.c -----------*/

VIDEO_START( mlc );
SCREEN_UPDATE( mlc );
SCREEN_EOF( mlc );
