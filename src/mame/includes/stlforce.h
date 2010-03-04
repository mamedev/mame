class stlforce_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, stlforce_state(machine)); }

	stlforce_state(running_machine &machine) { }

	tilemap_t *bg_tilemap;
	tilemap_t *mlow_tilemap;
	tilemap_t *mhigh_tilemap;
	tilemap_t *tx_tilemap;

	UINT16 *bg_videoram;
	UINT16 *mlow_videoram;
	UINT16 *mhigh_videoram;
	UINT16 *tx_videoram;
	UINT16 *bg_scrollram;
	UINT16 *mlow_scrollram;
	UINT16 *mhigh_scrollram;
	UINT16 *vidattrram;

	UINT16 *spriteram;

	int sprxoffs;
};


/*----------- defined in video/stlforce.c -----------*/

VIDEO_START( stlforce );
VIDEO_UPDATE( stlforce );
WRITE16_HANDLER( stlforce_tx_videoram_w );
WRITE16_HANDLER( stlforce_mhigh_videoram_w );
WRITE16_HANDLER( stlforce_mlow_videoram_w );
WRITE16_HANDLER( stlforce_bg_videoram_w );
