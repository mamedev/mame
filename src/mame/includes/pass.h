class pass_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, pass_state(machine)); }

	pass_state(running_machine &machine)
		: driver_data_t(machine) { }

	tilemap_t *bg_tilemap;
	tilemap_t *fg_tilemap;

	UINT16 *bg_videoram;
	UINT16 *fg_videoram;
};


/*----------- defined in video/pass.c -----------*/

WRITE16_HANDLER( pass_fg_videoram_w );
WRITE16_HANDLER( pass_bg_videoram_w );

VIDEO_START( pass );
VIDEO_UPDATE( pass );
