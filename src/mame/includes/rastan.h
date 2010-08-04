/*************************************************************************

    Rastan

*************************************************************************/

class rastan_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, rastan_state(machine)); }

	rastan_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
//  UINT16 *    paletteram; // this currently uses generic palette handlers

	/* video-related */
	UINT16      sprite_ctrl;
	UINT16      sprites_flipscreen;

	/* misc */
	int         adpcm_pos;
	int         adpcm_data;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *pc090oj;
	running_device *pc080sn;
};


/*----------- defined in video/rastan.c -----------*/

WRITE16_HANDLER( rastan_spritectrl_w );

VIDEO_UPDATE( rastan );
