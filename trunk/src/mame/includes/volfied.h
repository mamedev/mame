/*************************************************************************

    Volfied

*************************************************************************/

class volfied_state : public driver_device
{
public:
	volfied_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *    m_video_ram;
	UINT8  *    m_cchip_ram;
//  UINT16 *    paletteram; // this currently uses generic palette handlers

	/* video-related */
	UINT16      m_video_ctrl;
	UINT16      m_video_mask;

	/* c-chip */
	UINT8       m_current_bank;
	UINT8       m_current_flag;
	UINT8       m_cc_port;
	UINT8       m_current_cmd;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_pc090oj;
	DECLARE_WRITE16_MEMBER(volfied_cchip_ctrl_w);
	DECLARE_WRITE16_MEMBER(volfied_cchip_bank_w);
	DECLARE_WRITE16_MEMBER(volfied_cchip_ram_w);
	DECLARE_READ16_MEMBER(volfied_cchip_ctrl_r);
	DECLARE_READ16_MEMBER(volfied_cchip_ram_r);
	DECLARE_READ16_MEMBER(volfied_video_ram_r);
	DECLARE_WRITE16_MEMBER(volfied_video_ram_w);
	DECLARE_WRITE16_MEMBER(volfied_video_ctrl_w);
	DECLARE_READ16_MEMBER(volfied_video_ctrl_r);
	DECLARE_WRITE16_MEMBER(volfied_video_mask_w);
	DECLARE_WRITE16_MEMBER(volfied_sprite_ctrl_w);
};


/*----------- defined in machine/volfied.c -----------*/

void volfied_cchip_init(running_machine &machine);
void volfied_cchip_reset(running_machine &machine);



/*----------- defined in video/volfied.c -----------*/



SCREEN_UPDATE_IND16( volfied );
VIDEO_START( volfied );
