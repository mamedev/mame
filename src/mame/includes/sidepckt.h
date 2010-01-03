typedef struct _sidepckt_state sidepckt_state;
struct _sidepckt_state
{
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
