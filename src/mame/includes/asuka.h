/*************************************************************************

    Asuka & Asuka  (+ Taito/Visco games on similar hardware)

*************************************************************************/

class asuka_state : public driver_device
{
public:
	asuka_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
//  UINT16 *    paletteram; // this currently uses generic palette handlers

	/* video-related */
	UINT16      m_video_ctrl;
	UINT16      m_video_mask;

	/* c-chip */
	int         m_current_round;
	int         m_current_bank;

	UINT8       m_cval[26];
	UINT8       m_cc_port;
	UINT8       m_restart_status;

	/* misc */
	int         m_adpcm_pos;
	int         m_adpcm_data;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_pc090oj;
	device_t *m_tc0100scn;
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_WRITE8_MEMBER(asuka_msm5205_address_w);
	DECLARE_READ16_MEMBER(cadash_share_r);
	DECLARE_WRITE16_MEMBER(cadash_share_w);
	DECLARE_WRITE16_MEMBER(asuka_spritectrl_w);
};



/*----------- defined in machine/bonzeadv.c -----------*/

READ16_HANDLER( bonzeadv_cchip_ctrl_r );
READ16_HANDLER( bonzeadv_cchip_ram_r );
WRITE16_HANDLER( bonzeadv_cchip_ctrl_w );
WRITE16_HANDLER( bonzeadv_cchip_bank_w );
WRITE16_HANDLER( bonzeadv_cchip_ram_w );


/*----------- defined in video/asuka.c -----------*/


SCREEN_UPDATE_IND16( asuka );
SCREEN_UPDATE_IND16( bonzeadv );
