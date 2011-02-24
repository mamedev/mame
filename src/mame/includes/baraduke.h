class baraduke_state : public driver_device
{
public:
	baraduke_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int inputport_selected;
	int counter;
	UINT8 *textram;
	UINT8 *videoram;
	UINT8 *spriteram;
	tilemap_t *tx_tilemap;
	tilemap_t *bg_tilemap[2];
	int xscroll[2];
	int yscroll[2];
	int copy_sprites;
};


/*----------- defined in video/baraduke.c -----------*/

VIDEO_START( baraduke );
SCREEN_UPDATE( baraduke );
SCREEN_EOF( baraduke );
READ8_HANDLER( baraduke_videoram_r );
WRITE8_HANDLER( baraduke_videoram_w );
READ8_HANDLER( baraduke_textram_r );
WRITE8_HANDLER( baraduke_textram_w );
WRITE8_HANDLER( baraduke_scroll0_w );
WRITE8_HANDLER( baraduke_scroll1_w );
READ8_HANDLER( baraduke_spriteram_r );
WRITE8_HANDLER( baraduke_spriteram_w );
PALETTE_INIT( baraduke );
