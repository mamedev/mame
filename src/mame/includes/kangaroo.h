/***************************************************************************

    Sun Electronics Kangaroo hardware

    driver by Ville Laitinen

***************************************************************************/

typedef struct _kangaroo_state kangaroo_state;
struct _kangaroo_state
{
	/* memory pointers */
	UINT8 *      video_control;

	/* video-related */
	UINT32       *videoram;

	/* misc */
	UINT8        clock;
};




/*----------- defined in video/kangaroo.c -----------*/

VIDEO_START( kangaroo );
VIDEO_UPDATE( kangaroo );

WRITE8_HANDLER( kangaroo_videoram_w );
WRITE8_HANDLER( kangaroo_video_control_w );
