class punchout_state : public driver_device
{
public:
	punchout_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int rp5c01_mode_sel;
	int rp5c01_mem[16*4];
	UINT8 *bg_top_videoram;
	UINT8 *bg_bot_videoram;
	UINT8 *armwrest_fg_videoram;
	UINT8 *spr1_videoram;
	UINT8 *spr2_videoram;
	UINT8 *spr1_ctrlram;
	UINT8 *spr2_ctrlram;
	UINT8 *palettebank;
	tilemap_t *bg_top_tilemap;
	tilemap_t *bg_bot_tilemap;
	tilemap_t *fg_tilemap;
	tilemap_t *spr1_tilemap;
	tilemap_t *spr1_tilemap_flipx;
	tilemap_t *spr2_tilemap;
	int palette_reverse_top;
	int palette_reverse_bot;
};


/*----------- defined in video/punchout.c -----------*/

WRITE8_HANDLER( punchout_bg_top_videoram_w );
WRITE8_HANDLER( punchout_bg_bot_videoram_w );
WRITE8_HANDLER( armwrest_fg_videoram_w );
WRITE8_HANDLER( punchout_spr1_videoram_w );
WRITE8_HANDLER( punchout_spr2_videoram_w );

VIDEO_START( punchout );
VIDEO_START( armwrest );
SCREEN_UPDATE( punchout );
SCREEN_UPDATE( armwrest );

DRIVER_INIT( punchout );
DRIVER_INIT( spnchout );
DRIVER_INIT( spnchotj );
DRIVER_INIT( armwrest );
