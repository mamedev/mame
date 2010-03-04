class sidepckt_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, sidepckt_state(machine)); }

	sidepckt_state(running_machine &machine) { }

	tilemap_t *bg_tilemap;
	UINT8 *colorram;
	UINT8 *videoram;
	size_t videoram_size;
	UINT8 *spriteram;
	size_t spriteram_size;
	int i8751_return;
	int current_ptr;
	int current_table;
	int in_math;
	int math_param;
};


/*----------- defined in video/sidepckt.c -----------*/

PALETTE_INIT( sidepckt );
VIDEO_START( sidepckt );
VIDEO_UPDATE( sidepckt );

WRITE8_HANDLER( sidepckt_flipscreen_w );
WRITE8_HANDLER( sidepckt_videoram_w );
WRITE8_HANDLER( sidepckt_colorram_w );
