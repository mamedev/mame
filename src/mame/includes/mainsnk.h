class mainsnk_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, mainsnk_state(machine)); }

	mainsnk_state(running_machine &machine)
		: driver_data_t(machine) { }

	tilemap_t *tx_tilemap;
	tilemap_t *bg_tilemap;
	UINT8 *spriteram;
	UINT8 *fgram;
	UINT8 *bgram;

	int sound_cpu_busy;
	UINT32 bg_tile_offset;
};


/*----------- defined in video/mainsnk.c -----------*/

WRITE8_HANDLER(mainsnk_c600_w);
WRITE8_HANDLER(mainsnk_fgram_w);
WRITE8_HANDLER(mainsnk_bgram_w);
VIDEO_START(mainsnk);
VIDEO_UPDATE(mainsnk);
