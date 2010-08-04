class sidearms_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, sidearms_state(machine)); }

	sidearms_state(running_machine &machine)
		: driver_data_t(machine) { }

	int gameid;

	UINT8 *videoram;
	UINT8 *colorram;
	UINT8 *bg_scrollx;
	UINT8 *bg_scrolly;
	UINT8 *tilerom;
	tilemap_t *bg_tilemap;
	tilemap_t *fg_tilemap;

	int bgon;
	int objon;
	int staron;
	int charon;
	int flipon;

	UINT32 hflop_74a_n;
	UINT32 hcount_191;
	UINT32 vcount_191;
	UINT32 latch_374;
};

/*----------- defined in video/sidearms.c -----------*/

WRITE8_HANDLER( sidearms_videoram_w );
WRITE8_HANDLER( sidearms_colorram_w );
WRITE8_HANDLER( sidearms_star_scrollx_w );
WRITE8_HANDLER( sidearms_star_scrolly_w );
WRITE8_HANDLER( sidearms_c804_w );
WRITE8_HANDLER( sidearms_gfxctrl_w );

VIDEO_START( sidearms );
VIDEO_UPDATE( sidearms );
VIDEO_EOF( sidearms );
