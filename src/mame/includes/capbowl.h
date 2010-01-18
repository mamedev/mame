/*************************************************************************

    Coors Light Bowling/Bowl-O-Rama hardware

*************************************************************************/

typedef struct _capbowl_state capbowl_state;
struct _capbowl_state
{
	/* memory pointers */
	UINT8 *  rowaddress;
//  UINT8 *  nvram; // currently this uses generic_nvram

	/* video-related */
	offs_t blitter_addr;

	/* input-related */
	UINT8 last_trackball_val[2];

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
};

/*----------- defined in video/capbowl.c -----------*/

VIDEO_START( capbowl );
VIDEO_UPDATE( capbowl );

WRITE8_HANDLER( bowlrama_blitter_w );
READ8_HANDLER( bowlrama_blitter_r );

WRITE8_HANDLER( capbowl_tms34061_w );
READ8_HANDLER( capbowl_tms34061_r );
