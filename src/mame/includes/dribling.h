/*************************************************************************

    Model Racing Dribbling hardware

*************************************************************************/



typedef struct _dribling_state dribling_state;
struct _dribling_state
{
	/* memory pointers */
	UINT8 *  videoram;
	UINT8 *  colorram;

	/* misc */
	UINT8    abca;
	UINT8    dr, ds, sh;
	UINT8    input_mux;
	UINT8    di;

	/* devices */
	const device_config *maincpu;
	const device_config *ppi_0;
	const device_config *ppi_1;
};


/*----------- defined in video/dribling.c -----------*/

PALETTE_INIT( dribling );
WRITE8_HANDLER( dribling_colorram_w );
VIDEO_UPDATE( dribling );
