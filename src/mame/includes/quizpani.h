class quizpani_state : public driver_device
{
public:
	quizpani_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *bg_videoram;
	UINT16 *txt_videoram;
	UINT16 *scrollreg;
	tilemap_t *bg_tilemap;
	tilemap_t *txt_tilemap;
	int bgbank;
	int txtbank;
};


/*----------- defined in video/quizpani.c -----------*/

WRITE16_HANDLER( quizpani_bg_videoram_w );
WRITE16_HANDLER( quizpani_txt_videoram_w );
WRITE16_HANDLER( quizpani_tilesbank_w );

VIDEO_START( quizpani );
SCREEN_UPDATE( quizpani );
