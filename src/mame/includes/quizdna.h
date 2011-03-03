class quizdna_state : public driver_device
{
public:
	quizdna_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *bg_ram;
	UINT8 *fg_ram;
	tilemap_t *bg_tilemap;
	tilemap_t *fg_tilemap;
	UINT8 bg_xscroll[2];
	int flipscreen;
	int video_enable;
};


/*----------- defined in video/quizdna.c -----------*/

VIDEO_START( quizdna );
SCREEN_UPDATE( quizdna );

WRITE8_HANDLER( quizdna_fg_ram_w );
WRITE8_HANDLER( quizdna_bg_ram_w );
WRITE8_HANDLER( quizdna_bg_yscroll_w );
WRITE8_HANDLER( quizdna_bg_xscroll_w );
WRITE8_HANDLER( quizdna_screen_ctrl_w );

WRITE8_HANDLER( paletteram_xBGR_RRRR_GGGG_BBBB_w );
