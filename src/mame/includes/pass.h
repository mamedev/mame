class pass_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, pass_state(machine)); }

	pass_state(running_machine &machine) { }

	tilemap_t *bg_tilemap;
	tilemap_t *fg_tilemap;

	UINT16 *bg_videoram;
	UINT16 *fg_videoram;
};


/*----------- defined in video/pass.c -----------*/

VIDEO_START( pass );
VIDEO_UPDATE( pass );
WRITE16_HANDLER( pass_fg_videoram_w );
WRITE16_HANDLER( pass_bg_videoram_w );
