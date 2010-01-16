/*************************************************************************

    Rastan

*************************************************************************/

typedef struct _rastan_state rastan_state;
struct _rastan_state
{
	/* memory pointers */
//  UINT16 *    paletteram; // this currently uses generic palette handlers

	/* video-related */
	UINT16      sprite_ctrl;
	UINT16      sprites_flipscreen;

	/* misc */
	int         adpcm_pos;
	int         adpcm_data;

	/* devices */
	const device_config *maincpu;
	const device_config *audiocpu;
	const device_config *pc090oj;
	const device_config *pc080sn;
};


/*----------- defined in video/rastan.c -----------*/

WRITE16_HANDLER( rastan_spritectrl_w );

VIDEO_UPDATE( rastan );
