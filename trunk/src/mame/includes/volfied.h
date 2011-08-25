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
};


/*----------- defined in machine/volfied.c -----------*/

void volfied_cchip_init(running_machine &machine);
void volfied_cchip_reset(running_machine &machine);

READ16_HANDLER( volfied_cchip_ctrl_r );
READ16_HANDLER( volfied_cchip_ram_r );
WRITE16_HANDLER( volfied_cchip_ctrl_w );
WRITE16_HANDLER( volfied_cchip_bank_w );
WRITE16_HANDLER( volfied_cchip_ram_w );


/*----------- defined in video/volfied.c -----------*/

WRITE16_HANDLER( volfied_sprite_ctrl_w );
WRITE16_HANDLER( volfied_video_ram_w );
WRITE16_HANDLER( volfied_video_ctrl_w );
WRITE16_HANDLER( volfied_video_mask_w );

READ16_HANDLER( volfied_video_ram_r );
READ16_HANDLER( volfied_video_ctrl_r );

SCREEN_UPDATE( volfied );
VIDEO_START( volfied );
