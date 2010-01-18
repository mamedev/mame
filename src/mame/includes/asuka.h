/*************************************************************************

    Asuka & Asuka  (+ Taito/Visco games on similar hardware)

*************************************************************************/

typedef struct _asuka_state asuka_state;
struct _asuka_state
{
	/* memory pointers */
//  UINT16 *    paletteram; // this currently uses generic palette handlers

	/* video-related */
	UINT16      video_ctrl;
	UINT16      video_mask;

	/* c-chip */
	int         current_round;
	int         current_bank;

	UINT8       cval[26];
	UINT8       cc_port;
	UINT8       restart_status;

	/* misc */
	int         adpcm_pos;
	int         adpcm_data;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *pc090oj;
	running_device *tc0100scn;
};



/*----------- defined in machine/bonzeadv.c -----------*/

READ16_HANDLER( bonzeadv_cchip_ctrl_r );
READ16_HANDLER( bonzeadv_cchip_ram_r );
WRITE16_HANDLER( bonzeadv_cchip_ctrl_w );
WRITE16_HANDLER( bonzeadv_cchip_bank_w );
WRITE16_HANDLER( bonzeadv_cchip_ram_w );


/*----------- defined in video/asuka.c -----------*/

WRITE16_HANDLER( asuka_spritectrl_w );

VIDEO_UPDATE( asuka );
VIDEO_UPDATE( bonzeadv );
