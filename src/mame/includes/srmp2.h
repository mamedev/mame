typedef struct _srmp2_state srmp2_state;
struct _srmp2_state
{
	int color_bank;
	int gfx_bank;

	int adpcm_bank;
	int adpcm_data;
	UINT32 adpcm_sptr;
	UINT32 adpcm_eptr;

	int port_select;

	union
	{
		UINT8 *u8;
		UINT16 *u16;
	} spriteram1, spriteram2, spriteram3;
};


/*----------- defined in video/srmp2.c -----------*/

PALETTE_INIT( srmp2 );
VIDEO_UPDATE( srmp2 );
PALETTE_INIT( srmp3 );
VIDEO_UPDATE( srmp3 );
VIDEO_UPDATE( mjyuugi );
