/*************************************************************************

    Asuka & Asuka  (+ Taito/Visco games on similar hardware)

*************************************************************************/

class asuka_state : public driver_device
{
public:
	asuka_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

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
	device_t *maincpu;
	device_t *audiocpu;
	device_t *pc090oj;
	device_t *tc0100scn;
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
