typedef struct _xyonix_state xyonix_state;
struct _xyonix_state
{
	UINT8 *vidram;
	tilemap_t *tilemap;

	int e0_data;
	int credits;
	int coins;
	int prev_coin;
};


/*----------- defined in video/xyonix.c -----------*/

PALETTE_INIT( xyonix );
WRITE8_HANDLER( xyonix_vidram_w );
VIDEO_START(xyonix);
VIDEO_UPDATE(xyonix);
