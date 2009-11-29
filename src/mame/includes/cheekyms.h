/*************************************************************************

    Cheeky Mouse

*************************************************************************/


typedef struct _cheekyms_state cheekyms_state;
struct _cheekyms_state
{
	/* memory pointers */
	UINT8 *        videoram;
	UINT8 *        spriteram;
	UINT8 *        port_80;

	/* video-related */
	tilemap        *cm_tilemap;
	bitmap_t       *bitmap_buffer;

	/* devices */
	const device_config *maincpu;
	const device_config *dac;
};


/*----------- defined in video/cheekyms.c -----------*/

PALETTE_INIT( cheekyms );
VIDEO_START( cheekyms );
VIDEO_UPDATE( cheekyms );
WRITE8_HANDLER( cheekyms_port_40_w );
WRITE8_HANDLER( cheekyms_port_80_w );
