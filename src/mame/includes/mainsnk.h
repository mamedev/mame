class mainsnk_state : public driver_device
{
public:
	mainsnk_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	tilemap_t *m_tx_tilemap;
	tilemap_t *m_bg_tilemap;
	UINT8 *m_spriteram;
	UINT8 *m_fgram;
	UINT8 *m_bgram;

	int m_sound_cpu_busy;
	UINT32 m_bg_tile_offset;
};


/*----------- defined in video/mainsnk.c -----------*/

PALETTE_INIT(mainsnk);
WRITE8_HANDLER(mainsnk_c600_w);
WRITE8_HANDLER(mainsnk_fgram_w);
WRITE8_HANDLER(mainsnk_bgram_w);
VIDEO_START(mainsnk);
SCREEN_UPDATE(mainsnk);
