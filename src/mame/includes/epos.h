/*************************************************************************

    Epos games

**************************************************************************/

typedef struct _epos_state epos_state;
struct _epos_state
{
	/* memory pointers */
	UINT8 *  videoram;
	size_t   videoram_size;

	/* video-related */
	UINT8    palette;

	/* misc */
	int      counter;
};


/*----------- defined in video/epos.c -----------*/

WRITE8_HANDLER( epos_port_1_w );
VIDEO_UPDATE( epos );
