class vastar_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, vastar_state(machine)); }

	vastar_state(running_machine &machine)
		: driver_data_t(machine) { }

	UINT8 *spriteram1;
	UINT8 *spriteram2;
	UINT8 *spriteram3;

	UINT8 *bg1videoram;
	UINT8 *bg2videoram;
	UINT8 *fgvideoram;
	UINT8 *bg1_scroll;
	UINT8 *bg2_scroll;
	UINT8 *sprite_priority;

	tilemap_t *fg_tilemap;
	tilemap_t *bg1_tilemap;
	tilemap_t *bg2_tilemap;

	UINT8 *sharedram;
};


/*----------- defined in video/vastar.c -----------*/

WRITE8_HANDLER( vastar_bg1videoram_w );
WRITE8_HANDLER( vastar_bg2videoram_w );
WRITE8_HANDLER( vastar_fgvideoram_w );
READ8_HANDLER( vastar_bg1videoram_r );
READ8_HANDLER( vastar_bg2videoram_r );

VIDEO_START( vastar );
VIDEO_UPDATE( vastar );
